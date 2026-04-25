#include "can/m_can.hpp"

#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <optional>
#include <string>
#include <type_traits>

#include "mapping.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
constexpr std::size_t k_filter_slots = 28U;

#define RU_POSIX_M_CAN_ID_VALUE(name) M_canId::name,
constexpr auto k_m_can_ids =
    std::array<M_canId, static_cast<std::size_t>(M_canId::COUNT)>{
        M_CAN_LIST(RU_POSIX_M_CAN_ID_VALUE)};
#undef RU_POSIX_M_CAN_ID_VALUE

constexpr M_canId k_default_m_can_id = k_m_can_ids[0U];

struct can_state {
  bool initialized{false};
  CanControllerConfig config{};
  std::optional<M_fifo> not_matching{};
  std::array<uint8_t, 2> priorities{};
  std::array<bool, 2> interrupts{};
  uint8_t error_priority{0U};
  bool error_interrupt{false};
  std::array<std::optional<M_filter>, k_filter_slots> filters{};
  std::array<bool, k_filter_slots> filter_enabled{};
  std::array<void (*)(CanMessageTs), 2> rx_callbacks{nullptr, nullptr};
  void (*txfull_callback)(){nullptr};
};

can_state& state(const M_canId id) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_STATE_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name: {                             \
      static can_state value{};                            \
      return value;                                        \
    }
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_STATE_CASE)
#undef RU_POSIX_M_CAN_STATE_CASE
    default: {
      static can_state value{};
      return value;
    }
  }
}

constexpr int fifo_index(const M_fifo fifo) noexcept {
  return fifo == M_fifo::FIFO1 ? 1 : 0;
}

constexpr bool controller_config_is_valid(
    const CanControllerConfig& controller_config) noexcept {
  return controller_config.standard_filter_count <= k_filter_slots &&
         controller_config.extended_filter_count <= k_filter_slots;
}

constexpr bool filter_slot_is_valid(const CanIdFormat format, const uint8_t id,
                                    const CanControllerConfig& config) noexcept {
  return format == CanIdFormat::Extended ? id < config.extended_filter_count
                                         : id < config.standard_filter_count;
}

std::string to_hex_payload(const CanFrameView& message) {
  static constexpr char k_hex[] = "0123456789ABCDEF";
  const auto len =
      message.len > k_can_classic_max_payload ? k_can_classic_max_payload : message.len;

  std::string payload;
  payload.reserve(static_cast<std::size_t>(len) * 2U);

  for (uint8_t index = 0U; index < len; ++index) {
    const auto byte = message.data[index];
    payload.push_back(k_hex[(byte >> 4U) & 0x0FU]);
    payload.push_back(k_hex[byte & 0x0FU]);
  }

  return payload;
}

result parse_fifo_len_result(const virtual_driver::Result& generated,
                             int& len) noexcept {
  if (!generated.ok || generated.value.empty()) {
    return result::RECOVERABLE_ERROR;
  }

  char* end{nullptr};
  const auto* value = trim_generated_value(generated);
  const auto parsed = std::strtol(value, &end, 10);
  while (end != nullptr && std::isspace(static_cast<unsigned char>(*end)) != 0) {
    ++end;
  }

  if (end == value || (end != nullptr && *end != '\0')) {
    return result::RECOVERABLE_ERROR;
  }

  len = static_cast<int>(parsed);
  return result::OK;
}

result parse_can_message_result(const virtual_driver::Result& generated,
                                CanMessageTs& message) noexcept {
  if (!generated.ok || generated.value.empty()) {
    return result::RECOVERABLE_ERROR;
  }

  const auto* value = trim_generated_value(generated);
  while (std::isspace(static_cast<unsigned char>(*value)) != 0) {
    ++value;
  }

  char id_buffer[32]{};
  char data_buffer[64]{};
  if (std::sscanf(value, "id=%31s data=%63s", id_buffer, data_buffer) != 2) {
    return result::RECOVERABLE_ERROR;
  }

  char* end{nullptr};
  const auto id = std::strtoul(id_buffer, &end, 0);
  if (end == id_buffer) {
    return result::RECOVERABLE_ERROR;
  }

  const auto hex_len = std::strlen(data_buffer);
  if ((hex_len % 2U) != 0U) {
    return result::RECOVERABLE_ERROR;
  }

  std::array<uint8_t, k_can_classic_max_payload> payload{};
  auto len = static_cast<uint8_t>(hex_len / 2U);
  if (len > k_can_classic_max_payload) {
    len = k_can_classic_max_payload;
  }

  for (uint8_t index = 0U; index < len; ++index) {
    unsigned value_byte{0U};
    if (std::sscanf(data_buffer + (index * 2U), "%2x", &value_byte) != 1) {
      return result::RECOVERABLE_ERROR;
    }

    payload[index] = static_cast<uint8_t>(value_byte);
  }

  CanMessage frame{};
  if (id <= k_can_standard_id_mask) {
    const auto make_message =
        can_message::classic_standard(static_cast<uint32_t>(id), payload.data(), len);
    if (!make_message.has_value()) {
      return result::RECOVERABLE_ERROR;
    }

    frame = make_message.value();
  } else {
    const auto make_message =
        can_message::classic_extended(static_cast<uint32_t>(id), payload.data(), len);
    if (!make_message.has_value()) {
      return result::RECOVERABLE_ERROR;
    }

    frame = make_message.value();
  }

  message = CanMessageTs{frame, std::optional<uint8_t>{}, static_cast<Timestamp>(0U)};
  return result::OK;
}

#define RU_POSIX_DEFINE_M_CAN_BACKEND(wrapper_name, api_name)                            \
  result wrapper_name##_init() noexcept {                                                \
    if (result_from_generated(generated_api().api_name##_set_filter_fifo(0, 0U, 0U)) != \
        result::OK) {                                                                    \
      return result::RECOVERABLE_ERROR;                                                  \
    }                                                                                    \
                                                                                         \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(1, 0U, 0U));\
  }                                                                                      \
                                                                                         \
  result wrapper_name##_read_fifo_len(const M_fifo fifo, int& len) noexcept {           \
    return parse_fifo_len_result(generated_api().api_name##_fifo_len(fifo_index(fifo)), \
                                 len);                                                   \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_read_message(const M_fifo fifo, CanMessageTs& message) noexcept {\
    return parse_can_message_result(                                                     \
        generated_api().api_name##_peek_fifo(fifo_index(fifo)), message);                \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_write_message(const CanFrameView& message) noexcept {            \
    return result_from_generated(                                                        \
        generated_api().api_name##_write(message.id.value, to_hex_payload(message)));    \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_clear_filter(const M_fifo fifo) noexcept {                       \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(             \
        fifo_index(fifo), 0U, 0U));                                                       \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_set_filter(const M_fifo fifo, const uint32_t filter_id,          \
                                   const uint32_t filter_mask) noexcept {                \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(             \
        fifo_index(fifo), filter_id, filter_mask));                                      \
  }

RU_POSIX_DEFINE_M_CAN_BACKEND(can0, can_0)
RU_POSIX_DEFINE_M_CAN_BACKEND(can1, can_1)
RU_POSIX_DEFINE_M_CAN_BACKEND(can2, can_2)
RU_POSIX_DEFINE_M_CAN_BACKEND(can3, can_3)

#undef RU_POSIX_DEFINE_M_CAN_BACKEND

result init_can(const M_canId id) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_INIT_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                              \
      return wrapper_name##_init();
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_INIT_CASE)
#undef RU_POSIX_M_CAN_INIT_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result read_fifo_len(const M_canId id, const M_fifo fifo, int& len) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_LEN_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                             \
      return wrapper_name##_read_fifo_len(fifo, len);
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_LEN_CASE)
#undef RU_POSIX_M_CAN_LEN_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result read_message(const M_canId id, const M_fifo fifo,
                    CanMessageTs& message) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_READ_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                              \
      return wrapper_name##_read_message(fifo, message);
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_READ_CASE)
#undef RU_POSIX_M_CAN_READ_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result write_message(const M_canId id, const CanFrameView& message) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_WRITE_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                               \
      return wrapper_name##_write_message(message);
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_WRITE_CASE)
#undef RU_POSIX_M_CAN_WRITE_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result clear_filter(const M_canId id, const M_fifo fifo) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_CLEAR_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                               \
      return wrapper_name##_clear_filter(fifo);
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_CLEAR_CASE)
#undef RU_POSIX_M_CAN_CLEAR_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result set_filter_on_backend(const M_canId id, const M_fifo fifo,
                             const uint32_t filter_id,
                             const uint32_t filter_mask) noexcept {
  switch (id) {
#define RU_POSIX_M_CAN_SET_CASE(enum_name, wrapper_name) \
    case M_canId::enum_name:                             \
      return wrapper_name##_set_filter(fifo, filter_id, filter_mask);
    RU_POSIX_M_CAN_MAP(RU_POSIX_M_CAN_SET_CASE)
#undef RU_POSIX_M_CAN_SET_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

CanIdFormat filter_format(const M_filter& filter) noexcept {
  return std::visit(
      [](const auto& config) noexcept -> CanIdFormat { return config.format; },
      filter.config);
}

bool translate_filter(const M_filter& filter, uint32_t& filter_id,
                      uint32_t& filter_mask) noexcept {
  return std::visit(
      [&](const auto& config) noexcept -> bool {
        using config_type = std::decay_t<decltype(config)>;

        if constexpr (std::is_same_v<config_type, M_Mask>) {
          if (!can_id_value_is_valid(config.format, config.id) ||
              !can_id_value_is_valid(config.format, config.mask)) {
            return false;
          }

          filter_id = config.id;
          filter_mask = config.mask;
          return true;
        } else {
          return false;
        }
      },
      filter.config);
}

result apply_filter(const M_canId id, const M_filter& filter,
                    const bool enabled) noexcept {
  if (!enabled) {
    return clear_filter(id, filter.fifo);
  }

  uint32_t filter_id{0U};
  uint32_t filter_mask{0U};
  if (!translate_filter(filter, filter_id, filter_mask)) {
    return result::UNRECOVERABLE_ERROR;
  }

  return set_filter_on_backend(id, filter.fifo, filter_id, filter_mask);
}
}  // namespace

expected::expected<CanMessageTs, result> M_canRx::read(M_fifo fifo) noexcept {
  return const_cast<M_can&>(m_can).read(fifo);
}

expected::expected<CanMessageTs, result> M_canRx::try_read(M_fifo fifo) noexcept {
  return const_cast<M_can&>(m_can).try_read(fifo);
}

result M_canTx::write(const CanFrameView& message) noexcept {
  return const_cast<M_can&>(m_can).write(message);
}

result M_canTx::try_write(const CanFrameView& message) noexcept {
  return const_cast<M_can&>(m_can).try_write(message);
}

M_can::M_can(const M_canId id) noexcept : m_id(id) {
}

result M_can::configure(const CanControllerConfig& config) noexcept {
  return configure(k_default_m_can_id, config);
}

result M_can::configure(const M_canId id,
                        const CanControllerConfig& config) noexcept {
  if (!controller_config_is_valid(config)) {
    return result::UNRECOVERABLE_ERROR;
  }
  if (state(id).initialized) {
    return result::RECOVERABLE_ERROR;
  }

  state(id).config = config;
  return result::OK;
}

CanControllerConfig M_can::configuration() noexcept {
  return configuration(k_default_m_can_id);
}

CanControllerConfig M_can::configuration(const M_canId id) noexcept {
  return state(id).config;
}

result M_can::start() noexcept {
  return result::OK;
}

result M_can::init() noexcept {
  const auto status = init_can(m_id);
  if (status != result::OK) {
    return status;
  }

  state(m_id).initialized = true;
  return result::OK;
}

result M_can::stop() noexcept {
  state(m_id).initialized = false;
  return result::OK;
}

expected::expected<CanMessageTs, result> M_can::read(M_fifo fifo) noexcept {
  if (!state(m_id).initialized) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  CanMessageTs message{};
  const auto status = read_message(m_id, fifo, message);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return message;
}

expected::expected<CanMessageTs, result> M_can::try_read(M_fifo fifo) noexcept {
  if (!state(m_id).initialized) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  int len{0};
  const auto status = read_fifo_len(m_id, fifo, len);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  if (len <= 0) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  return read(fifo);
}

result M_can::write(const CanFrameView& message) noexcept {
  if (!state(m_id).initialized) {
    return result::RECOVERABLE_ERROR;
  }
  if (!message.is_valid() || message.frame_format != CanFrameFormat::Classic ||
      message.len > k_can_classic_max_payload) {
    return result::UNRECOVERABLE_ERROR;
  }

  return write_message(m_id, message);
}

result M_can::try_write(const CanFrameView& message) noexcept {
  return write(message);
}

result M_can::set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs)) {
  state(m_id).rx_callbacks[static_cast<std::size_t>(fifo_index(fifo))] = callback;
  return result::OK;
}

result M_can::set_txfull_callback(void (*callback)()) {
  state(m_id).txfull_callback = callback;
  return result::OK;
}

result M_can::set_not_matching(std::optional<M_fifo> fifo) {
  state(m_id).not_matching = fifo;
  return result::OK;
}

expected::expected<std::optional<M_fifo>, result> M_can::get_not_matching() {
  return state(m_id).not_matching;
}

result M_can::reset_timestamp() {
  return result::OK;
}

expected::expected<Timestamp, result> M_can::get_timestamp() {
  return static_cast<Timestamp>(0U);
}

result M_can::set_priority(M_fifo fifo, uint8_t priority) {
  state(m_id).priorities[static_cast<std::size_t>(fifo_index(fifo))] = priority;
  return result::OK;
}

expected::expected<uint8_t, result> M_can::get_priority(M_fifo fifo) {
  return state(m_id).priorities[static_cast<std::size_t>(fifo_index(fifo))];
}

result M_can::set_interrupt(M_fifo fifo, bool on) {
  state(m_id).interrupts[static_cast<std::size_t>(fifo_index(fifo))] = on;
  return result::OK;
}

expected::expected<uint8_t, result> M_can::is_interrupt_on(M_fifo fifo) {
  return static_cast<uint8_t>(
      state(m_id).interrupts[static_cast<std::size_t>(fifo_index(fifo))]);
}

result M_can::set_error_priority(uint8_t priority) {
  state(m_id).error_priority = priority;
  return result::OK;
}

expected::expected<uint8_t, result> M_can::get_error_priority() {
  return state(m_id).error_priority;
}

result M_can::set_error_interrupt(bool on) {
  state(m_id).error_interrupt = on;
  return result::OK;
}

expected::expected<uint8_t, result> M_can::is_error_interrupt_on() {
  return static_cast<uint8_t>(state(m_id).error_interrupt);
}

result M_can::set_filter(M_filter filter, uint8_t id) {
  if (!filter_slot_is_valid(filter_format(filter), id, state(m_id).config)) {
    return result::UNRECOVERABLE_ERROR;
  }

  const auto status = apply_filter(m_id, filter, true);
  if (status != result::OK) {
    return status;
  }

  auto& controller_state = state(m_id);
  controller_state.filters[id] = filter;
  controller_state.filter_enabled[id] = true;
  return result::OK;
}

result M_can::enable_filter(uint8_t id) {
  if (id >= k_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& controller_state = state(m_id);
  if (!controller_state.filters[id].has_value()) {
    return result::RECOVERABLE_ERROR;
  }
  if (!filter_slot_is_valid(filter_format(controller_state.filters[id].value()), id,
                            controller_state.config)) {
    return result::UNRECOVERABLE_ERROR;
  }

  const auto status = apply_filter(m_id, controller_state.filters[id].value(), true);
  if (status != result::OK) {
    return status;
  }

  controller_state.filter_enabled[id] = true;
  return result::OK;
}

result M_can::disable_filter(uint8_t id) {
  if (id >= k_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& controller_state = state(m_id);
  if (!controller_state.filters[id].has_value()) {
    return result::RECOVERABLE_ERROR;
  }
  if (!filter_slot_is_valid(filter_format(controller_state.filters[id].value()), id,
                            controller_state.config)) {
    return result::UNRECOVERABLE_ERROR;
  }

  const auto status = apply_filter(m_id, controller_state.filters[id].value(), false);
  if (status != result::OK) {
    return status;
  }

  controller_state.filter_enabled[id] = false;
  return result::OK;
}

expected::expected<bool, result> M_can::is_filter_enabled(uint8_t id) {
  if (id >= k_filter_slots) {
    return expected::unexpected(result::UNRECOVERABLE_ERROR);
  }

  return state(m_id).filter_enabled[id];
}

result M_canRx::set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs)) {
  return const_cast<M_can&>(m_can).set_rx_callback(fifo, callback);
}

result M_canRx::set_not_matching(std::optional<M_fifo> fifo) {
  return const_cast<M_can&>(m_can).set_not_matching(fifo);
}

expected::expected<std::optional<M_fifo>, result> M_canRx::get_not_matching() {
  return const_cast<M_can&>(m_can).get_not_matching();
}

result M_canRx::reset_timestamp() {
  return const_cast<M_can&>(m_can).reset_timestamp();
}

expected::expected<Timestamp, result> M_canRx::get_timestamp() {
  return const_cast<M_can&>(m_can).get_timestamp();
}

result M_canRx::set_priority(M_fifo fifo, uint8_t priority) {
  return const_cast<M_can&>(m_can).set_priority(fifo, priority);
}

expected::expected<uint8_t, result> M_canRx::get_priority(M_fifo fifo) {
  return const_cast<M_can&>(m_can).get_priority(fifo);
}

result M_canRx::set_interrupt(M_fifo fifo, bool on) {
  return const_cast<M_can&>(m_can).set_interrupt(fifo, on);
}

expected::expected<uint8_t, result> M_canRx::is_interrupt_on(M_fifo fifo) {
  return const_cast<M_can&>(m_can).is_interrupt_on(fifo);
}

result M_canRx::set_error_priority(uint8_t priority) {
  return const_cast<M_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> M_canRx::get_error_priority() {
  return const_cast<M_can&>(m_can).get_error_priority();
}

result M_canRx::set_error_interrupt(bool on) {
  return const_cast<M_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> M_canRx::is_error_interrupt_on() {
  return const_cast<M_can&>(m_can).is_error_interrupt_on();
}

result M_canRx::set_filter(M_filter filter, uint8_t id) {
  return const_cast<M_can&>(m_can).set_filter(filter, id);
}

result M_canRx::enable_filter(uint8_t id) {
  return const_cast<M_can&>(m_can).enable_filter(id);
}

result M_canRx::disable_filter(uint8_t id) {
  return const_cast<M_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> M_canRx::is_filter_enabled(uint8_t id) {
  return const_cast<M_can&>(m_can).is_filter_enabled(id);
}

result M_canTx::set_txfull_callback(void (*callback)()) {
  return const_cast<M_can&>(m_can).set_txfull_callback(callback);
}

result M_canTx::reset_timestamp() {
  return const_cast<M_can&>(m_can).reset_timestamp();
}

expected::expected<Timestamp, result> M_canTx::get_timestamp() {
  return const_cast<M_can&>(m_can).get_timestamp();
}

result M_canTx::set_error_priority(uint8_t priority) {
  return const_cast<M_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> M_canTx::get_error_priority() {
  return const_cast<M_can&>(m_can).get_error_priority();
}

result M_canTx::set_error_interrupt(bool on) {
  return const_cast<M_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> M_canTx::is_error_interrupt_on() {
  return const_cast<M_can&>(m_can).is_error_interrupt_on();
}
}  // namespace ru::driver
