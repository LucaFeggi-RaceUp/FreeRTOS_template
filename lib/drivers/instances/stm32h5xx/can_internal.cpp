#include "can_internal.hpp"

#include <algorithm>
#include <cstring>
#include <type_traits>

using namespace ru::driver;

namespace ru::driver::can_internal {
namespace {
constexpr uint32_t hw_fifo(const M_fifo fifo) noexcept {
  return fifo == M_fifo::FIFO1 ? FDCAN_RX_FIFO1 : FDCAN_RX_FIFO0;
}

constexpr uint32_t global_filter_target(const std::optional<M_fifo>& fifo) noexcept {
  if (!fifo.has_value()) {
    return FDCAN_REJECT;
  }

  return fifo.value() == M_fifo::FIFO1 ? FDCAN_ACCEPT_IN_RX_FIFO1 : FDCAN_ACCEPT_IN_RX_FIFO0;
}

constexpr uint32_t fifo_filter_target(const M_fifo fifo) noexcept {
  return fifo == M_fifo::FIFO1 ? FDCAN_FILTER_TO_RXFIFO1 : FDCAN_FILTER_TO_RXFIFO0;
}

constexpr uint32_t normalize_bx16_value(const uint16_t value) noexcept {
  return value <= 0x7FFU ? value : ((value >> 5U) & 0x7FFU);
}

CanMessageTs make_message_ts(const FDCAN_RxHeaderTypeDef& header,
                            const uint8_t* const payload) noexcept {
  CanMessage message{};
  message.id = header.Identifier & 0x7FFU;
  message.len = fdcan_length_from_dlc(header.DataLength);
  message.full_word = 0U;
  std::memcpy(message.bytes, payload, message.len);

  const auto filter =
      header.IsFilterMatchingFrame == 0U
          ? std::optional<uint8_t>{static_cast<uint8_t>(header.FilterIndex)}
          : std::optional<uint8_t>{};
  return CanMessageTs{message, filter, static_cast<Timestamp>(header.RxTimestamp)};
}

FDCAN_HandleTypeDef g_fdcan1_handle{};
bool g_fdcan1_initialized{false};
bool g_fdcan1_started{false};
std::array<uint8_t, 2> g_fdcan1_rx_priorities{k_default_irq_priority,
                                              k_default_irq_priority};
std::array<bool, 2> g_fdcan1_rx_interrupts{false, false};
uint8_t g_fdcan1_error_priority{k_default_irq_priority};
bool g_fdcan1_error_interrupt{false};
std::optional<M_fifo> g_fdcan1_not_matching{};
std::array<std::optional<M_filter>, k_std_filter_slots> g_fdcan1_m_filters{};
std::array<bool, k_std_filter_slots> g_fdcan1_m_filter_enabled{};
std::array<std::optional<Bx_filter>, k_std_filter_slots> g_fdcan1_bx_filters{};
std::array<bool, k_std_filter_slots> g_fdcan1_bx_filter_enabled{};
std::array<void (*)(CanMessageTs), 2> g_fdcan1_rx_callbacks{nullptr, nullptr};
void (*g_fdcan1_txfull_callback)(){nullptr};

void update_irq_priorities(const opaque_can& config) noexcept {
  const auto& fifo_priorities = rx_priorities(config);
  const auto line0_priority = fifo_priorities[0];
  const auto line1_priority = std::min(fifo_priorities[1], error_priority(config));

  HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, line0_priority, 0U);
  HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, line1_priority, 0U);
}

result do_apply_global_filter(const opaque_can& config) noexcept {
  return from_hal_status(HAL_FDCAN_ConfigGlobalFilter(
      &handle(config), global_filter_target(not_matching(config)),
      global_filter_target(not_matching(config)), FDCAN_REJECT_REMOTE,
      FDCAN_REJECT_REMOTE));
}

bool translate_m_filter(const M_filter& filter, FDCAN_FilterTypeDef& hw_filter) noexcept {
  hw_filter.IdType = FDCAN_STANDARD_ID;
  hw_filter.FilterConfig = fifo_filter_target(filter.fifo);

  return std::visit(
      [&](const auto& config) noexcept -> bool {
        using config_type = std::decay_t<decltype(config)>;

        if constexpr (std::is_same_v<config_type, M_Mask>) {
          hw_filter.FilterType = FDCAN_FILTER_MASK;
          hw_filter.FilterID1 = config.id & 0x7FFU;
          hw_filter.FilterID2 = config.mask & 0x7FFU;
          return true;
        } else if constexpr (std::is_same_v<config_type, M_Dual>) {
          hw_filter.FilterType = FDCAN_FILTER_DUAL;
          hw_filter.FilterID1 = config.id1 & 0x7FFU;
          hw_filter.FilterID2 = config.id2 & 0x7FFU;
          return true;
        } else if constexpr (std::is_same_v<config_type, M_Range>) {
          hw_filter.FilterType = FDCAN_FILTER_RANGE;
          hw_filter.FilterID1 = config.from & 0x7FFU;
          hw_filter.FilterID2 = config.to & 0x7FFU;
          return true;
        } else {
          return false;
        }
      },
      filter.config);
}

bool translate_bx_filter(const Bx_filter& filter, FDCAN_FilterTypeDef& hw_filter) noexcept {
  hw_filter.IdType = FDCAN_STANDARD_ID;
  hw_filter.FilterConfig = fifo_filter_target(to_m_fifo(filter.fifo));

  return std::visit(
      [&](const auto& config) noexcept -> bool {
        using config_type = std::decay_t<decltype(config)>;

        if constexpr (std::is_same_v<config_type, Bx_Mask32>) {
          hw_filter.FilterType = FDCAN_FILTER_MASK;
          hw_filter.FilterID1 = config.id & 0x7FFU;
          hw_filter.FilterID2 = config.mask & 0x7FFU;
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_List32>) {
          hw_filter.FilterType = FDCAN_FILTER_DUAL;
          hw_filter.FilterID1 = config.id1 & 0x7FFU;
          hw_filter.FilterID2 = config.id2 & 0x7FFU;
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_Mask16>) {
          hw_filter.FilterType = FDCAN_FILTER_MASK;
          hw_filter.FilterID1 = normalize_bx16_value(config.id1);
          hw_filter.FilterID2 = normalize_bx16_value(config.mask1);
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_List16>) {
          hw_filter.FilterType = FDCAN_FILTER_DUAL;
          hw_filter.FilterID1 = normalize_bx16_value(config.id1);
          hw_filter.FilterID2 = normalize_bx16_value(config.id2);
          return true;
        } else {
          return false;
        }
      },
      filter.config);
}
}  // namespace

FDCAN_HandleTypeDef& handle(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_handle;
}

bool& initialized(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_initialized;
}

bool& started(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_started;
}

std::array<uint8_t, 2>& rx_priorities(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_rx_priorities;
}

std::array<bool, 2>& rx_interrupts(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_rx_interrupts;
}

uint8_t& error_priority(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_error_priority;
}

bool& error_interrupt(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_error_interrupt;
}

std::optional<M_fifo>& not_matching(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_not_matching;
}

std::array<std::optional<M_filter>, k_std_filter_slots>& m_filters(
    const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_m_filters;
}

std::array<bool, k_std_filter_slots>& m_filter_enabled(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_m_filter_enabled;
}

std::array<std::optional<Bx_filter>, k_std_filter_slots>& bx_filters(
    const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_bx_filters;
}

std::array<bool, k_std_filter_slots>& bx_filter_enabled(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_bx_filter_enabled;
}

std::array<void (*)(CanMessageTs), 2>& rx_callbacks(const opaque_can& config) noexcept {
  (void)config;
  return g_fdcan1_rx_callbacks;
}

void (*&txfull_callback(const opaque_can& config))() {
  (void)config;
  return g_fdcan1_txfull_callback;
}
result apply_global_filter(const opaque_can& config) noexcept {
  return do_apply_global_filter(config);
}

result refresh_notifications(const opaque_can& config) noexcept {
  update_irq_priorities(config);

  if (!initialized(config) || !started(config)) {
    return result::OK;
  }

  auto& hw_handle = handle(config);
  if (HAL_FDCAN_DeactivateNotification(&hw_handle, k_all_interrupt_mask) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (rx_interrupts(config)[0] || rx_callbacks(config)[0] != nullptr) {
    if (HAL_FDCAN_ActivateNotification(&hw_handle, k_rx_interrupt_mask_fifo0, 0U) != HAL_OK) {
      return result::RECOVERABLE_ERROR;
    }
  }

  if (rx_interrupts(config)[1] || rx_callbacks(config)[1] != nullptr) {
    if (HAL_FDCAN_ActivateNotification(&hw_handle, k_rx_interrupt_mask_fifo1, 0U) != HAL_OK) {
      return result::RECOVERABLE_ERROR;
    }
  }

  if (error_interrupt(config)) {
    if (HAL_FDCAN_ActivateNotification(&hw_handle, k_error_interrupt_mask, 0U) != HAL_OK) {
      return result::RECOVERABLE_ERROR;
    }
  }

  if (txfull_callback(config) != nullptr) {
    if (HAL_FDCAN_ActivateNotification(&hw_handle, k_tx_interrupt_mask, FDCAN_TX_BUFFER0) !=
        HAL_OK) {
      return result::RECOVERABLE_ERROR;
    }
  }

  HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
  return result::OK;
}

result init_controller(const opaque_can& config) noexcept {
  if (config.m_p_instance == nullptr || config.m_p_port_rx == nullptr ||
      config.m_p_port_tx == nullptr) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& hw_handle = handle(config);
  if (initialized(config)) {
    if (!started(config)) {
      if (HAL_FDCAN_Start(&hw_handle) != HAL_OK) {
        return result::RECOVERABLE_ERROR;
      }
      started(config) = true;
    }

    return refresh_notifications(config);
  }

  enable_fdcan_clock(config.m_p_instance);
  init_alternate_pin(config.m_p_port_rx, config.m_rx_pin, config.m_alternate);
  init_alternate_pin(config.m_p_port_tx, config.m_tx_pin, config.m_alternate);

  hw_handle = {};
  hw_handle.Instance = config.m_p_instance;
  hw_handle.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hw_handle.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hw_handle.Init.Mode = FDCAN_MODE_NORMAL;
  hw_handle.Init.AutoRetransmission = ENABLE;
  hw_handle.Init.TransmitPause = DISABLE;
  hw_handle.Init.ProtocolException = DISABLE;
  hw_handle.Init.NominalPrescaler = 16U;
  hw_handle.Init.NominalSyncJumpWidth = 1U;
  hw_handle.Init.NominalTimeSeg1 = 2U;
  hw_handle.Init.NominalTimeSeg2 = 2U;
  hw_handle.Init.DataPrescaler = 1U;
  hw_handle.Init.DataSyncJumpWidth = 1U;
  hw_handle.Init.DataTimeSeg1 = 1U;
  hw_handle.Init.DataTimeSeg2 = 1U;
  hw_handle.Init.StdFiltersNbr = static_cast<uint32_t>(k_std_filter_slots);
  hw_handle.Init.ExtFiltersNbr = 0U;
  hw_handle.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

  if (HAL_FDCAN_Init(&hw_handle) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (HAL_FDCAN_ConfigTimestampCounter(&hw_handle, 1U) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (apply_global_filter(config) != result::OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (HAL_FDCAN_ConfigInterruptLines(&hw_handle, FDCAN_IT_GROUP_RX_FIFO0,
                                     FDCAN_INTERRUPT_LINE0) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (HAL_FDCAN_ConfigInterruptLines(
          &hw_handle,
          FDCAN_IT_GROUP_RX_FIFO1 | FDCAN_IT_GROUP_SMSG | FDCAN_IT_GROUP_MISC |
              FDCAN_IT_GROUP_BIT_LINE_ERROR | FDCAN_IT_GROUP_PROTOCOL_ERROR,
          FDCAN_INTERRUPT_LINE1) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  update_irq_priorities(config);
  HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
  HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);

  if (HAL_FDCAN_Start(&hw_handle) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  initialized(config) = true;
  started(config) = true;
  return refresh_notifications(config);
}

result stop_controller(const opaque_can& config) noexcept {
  if (!initialized(config) || !started(config)) {
    return result::OK;
  }

  auto& hw_handle = handle(config);
  if (HAL_FDCAN_DeactivateNotification(&hw_handle, k_all_interrupt_mask) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  if (HAL_FDCAN_Stop(&hw_handle) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  started(config) = false;
  return result::OK;
}

expected::expected<CanMessageTs, result> read_fifo_message(const opaque_can& config,
                                                          const M_fifo fifo) noexcept {
  if (!initialized(config) || !started(config)) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  auto& hw_handle = handle(config);
  if (HAL_FDCAN_GetRxFifoFillLevel(&hw_handle, hw_fifo(fifo)) == 0U) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  FDCAN_RxHeaderTypeDef header{};
  uint8_t payload[8]{};
  if (HAL_FDCAN_GetRxMessage(&hw_handle, hw_fifo(fifo), &header, payload) != HAL_OK) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  return make_message_ts(header, payload);
}

result write_controller_message(const opaque_can& config,
                                const CanMessage& message) noexcept {
  if (!initialized(config) || !started(config)) {
    return result::RECOVERABLE_ERROR;
  }

  FDCAN_TxHeaderTypeDef header{};
  header.Identifier = message.id & 0x7FFU;
  header.IdType = FDCAN_STANDARD_ID;
  header.TxFrameType = FDCAN_DATA_FRAME;
  header.DataLength = fdcan_dlc_from_length(static_cast<uint8_t>(message.len));
  header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  header.BitRateSwitch = FDCAN_BRS_OFF;
  header.FDFormat = FDCAN_CLASSIC_CAN;
  header.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  header.MessageMarker = 0U;

  uint8_t payload[8]{};
  const auto len = static_cast<uint8_t>(message.len > 8U ? 8U : message.len);
  std::memcpy(payload, message.bytes, len);
  return from_hal_status(HAL_FDCAN_AddMessageToTxFifoQ(&handle(config), &header, payload));
}

result configure_m_filter(const opaque_can& config, const M_filter& filter, const uint8_t id,
                          const bool enabled) noexcept {
  if (id >= k_std_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  FDCAN_FilterTypeDef hw_filter{};
  hw_filter.FilterIndex = id;
  if (!translate_m_filter(filter, hw_filter)) {
    return result::UNRECOVERABLE_ERROR;
  }

  if (!enabled) {
    hw_filter.FilterConfig = FDCAN_FILTER_DISABLE;
  }

  return from_hal_status(HAL_FDCAN_ConfigFilter(&handle(config), &hw_filter));
}

result configure_bx_filter(const opaque_can& config, const Bx_filter& filter, const uint8_t id,
                           const bool enabled) noexcept {
  if (id >= k_std_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  FDCAN_FilterTypeDef hw_filter{};
  hw_filter.FilterIndex = id;
  if (!translate_bx_filter(filter, hw_filter)) {
    return result::UNRECOVERABLE_ERROR;
  }

  if (!enabled) {
    hw_filter.FilterConfig = FDCAN_FILTER_DISABLE;
  }

  return from_hal_status(HAL_FDCAN_ConfigFilter(&handle(config), &hw_filter));
}

void service_rx_fifo(const opaque_can& config, const M_fifo fifo) noexcept {
  const auto callback = rx_callbacks(config)[fifo_index(fifo)];
  if (callback == nullptr) {
    return;
  }

  while (HAL_FDCAN_GetRxFifoFillLevel(&handle(config), hw_fifo(fifo)) != 0U) {
    const auto message = read_fifo_message(config, fifo);
    if (!message.has_value()) {
      break;
    }

    callback(message.value());
  }
}
}  // namespace ru::driver::can_internal

extern "C" void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef* hfdcan,
                                          uint32_t rx_fifo0_its) {
  (void)rx_fifo0_its;
  const auto config = ru::driver::can_internal::make_opaque(ru::driver::M_canId::CAN_2);
  if (hfdcan == &ru::driver::can_internal::handle(config)) {
    ru::driver::can_internal::service_rx_fifo(config, ru::driver::M_fifo::FIFO0);
  }
}

extern "C" void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef* hfdcan,
                                          uint32_t rx_fifo1_its) {
  (void)rx_fifo1_its;
  const auto config = ru::driver::can_internal::make_opaque(ru::driver::M_canId::CAN_2);
  if (hfdcan == &ru::driver::can_internal::handle(config)) {
    ru::driver::can_internal::service_rx_fifo(config, ru::driver::M_fifo::FIFO1);
  }
}

extern "C" void HAL_FDCAN_TxBufferCompleteCallback(FDCAN_HandleTypeDef* hfdcan,
                                                   uint32_t buffer_indexes) {
  (void)buffer_indexes;
  const auto config = ru::driver::can_internal::make_opaque(ru::driver::M_canId::CAN_2);
  if (hfdcan == &ru::driver::can_internal::handle(config) &&
      ru::driver::can_internal::txfull_callback(config) != nullptr) {
    ru::driver::can_internal::txfull_callback(config)();
  }
}

extern "C" void FDCAN1_IT0_IRQHandler(void) {
  auto& hw_handle = ru::driver::can_internal::handle(
      ru::driver::can_internal::make_opaque(ru::driver::M_canId::CAN_2));
  HAL_FDCAN_IRQHandler(&hw_handle);
}

extern "C" void FDCAN1_IT1_IRQHandler(void) {
  auto& hw_handle = ru::driver::can_internal::handle(
      ru::driver::can_internal::make_opaque(ru::driver::M_canId::CAN_2));
  HAL_FDCAN_IRQHandler(&hw_handle);
}
