#include "can/bx_can.hpp"

#include <array>
#include <cctype>
#include <cstdio>
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

struct can_state {
  bool initialized{false};
  std::array<uint8_t, 2> priorities{};
  std::array<bool, 2> interrupts{};
  uint8_t error_priority{0U};
  bool error_interrupt{false};
  std::array<std::optional<Bx_filter>, k_filter_slots> filters{};
  std::array<bool, k_filter_slots> filter_enabled{};
};

can_state& state(const Bx_canId id) noexcept {
  switch (id) {
    case Bx_canId::CAN_1: {
      static can_state value{};
      return value;
    }
    case Bx_canId::CAN_2: {
      static can_state value{};
      return value;
    }
    default: {
      static can_state value{};
      return value;
    }
  }
}

constexpr int fifo_index(const Bx_fifo fifo) noexcept {
  return fifo == Bx_fifo::FIFO1 ? 1 : 0;
}

std::string to_hex_payload(const CanFrameView& message) {
  static constexpr char k_hex[] = "0123456789ABCDEF";
  const auto len =
      message.len > k_can_classic_max_payload ? k_can_classic_max_payload : message.len;

  std::string payload;
  payload.reserve(static_cast<size_t>(len) * 2U);

  for (uint8_t index = 0U; index < len; ++index) {
    const auto byte = message.data[index];
    payload.push_back(k_hex[(byte >> 4U) & 0x0FU]);
    payload.push_back(k_hex[byte & 0x0FU]);
  }

  return payload;
}

result parse_fifo_len_result(const virtual_driver::Result& generated, int& len) noexcept {
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
                                CanMessage& message) noexcept {
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

  if (id <= k_can_standard_id_mask) {
    const auto make_message =
        can_message::classic_standard(static_cast<uint32_t>(id), payload.data(), len);
    if (!make_message.has_value()) {
      return result::RECOVERABLE_ERROR;
    }

    message = make_message.value();
    return result::OK;
  }

  const auto make_message =
      can_message::classic_extended(static_cast<uint32_t>(id), payload.data(), len);
  if (!make_message.has_value()) {
    return result::RECOVERABLE_ERROR;
  }

  message = make_message.value();
  return result::OK;
}

#define RU_POSIX_DEFINE_BX_CAN_BACKEND(wrapper_name, api_name)                           \
  result wrapper_name##_init() noexcept {                                                \
    if (result_from_generated(generated_api().api_name##_set_filter_fifo(0, 0U, 0U)) != \
        result::OK) {                                                                    \
      return result::RECOVERABLE_ERROR;                                                  \
    }                                                                                    \
                                                                                         \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(1, 0U, 0U));\
  }                                                                                      \
                                                                                         \
  result wrapper_name##_read_fifo_len(const Bx_fifo fifo, int& len) noexcept {          \
    return parse_fifo_len_result(generated_api().api_name##_fifo_len(fifo_index(fifo)), \
                                 len);                                                   \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_read_message(const Bx_fifo fifo, CanMessage& message) noexcept { \
    return parse_can_message_result(                                                     \
        generated_api().api_name##_peek_fifo(fifo_index(fifo)), message);                \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_write_message(const CanFrameView& message) noexcept {            \
    return result_from_generated(                                                        \
        generated_api().api_name##_write(message.id.value, to_hex_payload(message)));    \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_clear_filter(const Bx_fifo fifo) noexcept {                      \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(             \
        fifo_index(fifo), 0U, 0U));                                                      \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_set_filter(const Bx_fifo fifo, const uint32_t filter_id,         \
                                   const uint32_t filter_mask) noexcept {                \
    return result_from_generated(generated_api().api_name##_set_filter_fifo(             \
        fifo_index(fifo), filter_id, filter_mask));                                      \
  }

RU_POSIX_DEFINE_BX_CAN_BACKEND(can0, can_0)
RU_POSIX_DEFINE_BX_CAN_BACKEND(can1, can_1)
RU_POSIX_DEFINE_BX_CAN_BACKEND(can2, can_2)
RU_POSIX_DEFINE_BX_CAN_BACKEND(can3, can_3)

#undef RU_POSIX_DEFINE_BX_CAN_BACKEND

result init_can(const Bx_canId id) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_INIT_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                              \
      return wrapper_name##_init();
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_INIT_CASE)
#undef RU_POSIX_BX_CAN_INIT_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result read_fifo_len(const Bx_canId id, const Bx_fifo fifo, int& len) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_LEN_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                             \
      return wrapper_name##_read_fifo_len(fifo, len);
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_LEN_CASE)
#undef RU_POSIX_BX_CAN_LEN_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result read_message(const Bx_canId id, const Bx_fifo fifo, CanMessage& message) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_READ_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                              \
      return wrapper_name##_read_message(fifo, message);
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_READ_CASE)
#undef RU_POSIX_BX_CAN_READ_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result write_message(const Bx_canId id, const CanFrameView& message) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_WRITE_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                               \
      return wrapper_name##_write_message(message);
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_WRITE_CASE)
#undef RU_POSIX_BX_CAN_WRITE_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result clear_filter(const Bx_canId id, const Bx_fifo fifo) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_CLEAR_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                               \
      return wrapper_name##_clear_filter(fifo);
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_CLEAR_CASE)
#undef RU_POSIX_BX_CAN_CLEAR_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result set_filter_on_backend(const Bx_canId id, const Bx_fifo fifo,
                             const uint32_t filter_id,
                             const uint32_t filter_mask) noexcept {
  switch (id) {
#define RU_POSIX_BX_CAN_SET_CASE(enum_name, wrapper_name) \
    case Bx_canId::enum_name:                             \
      return wrapper_name##_set_filter(fifo, filter_id, filter_mask);
    RU_POSIX_BX_CAN_MAP(RU_POSIX_BX_CAN_SET_CASE)
#undef RU_POSIX_BX_CAN_SET_CASE
    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

constexpr uint32_t normalize_bx16_value(const uint16_t value) noexcept {
  return value <= 0x7FFU ? value : ((value >> 5U) & 0x7FFU);
}

bool translate_filter(const Bx_filter& filter, uint32_t& filter_id,
                      uint32_t& filter_mask) noexcept {
  return std::visit(
      [&](const auto& config) noexcept -> bool {
        using config_type = std::decay_t<decltype(config)>;

        if constexpr (std::is_same_v<config_type, Bx_Mask32>) {
          filter_id = config.id & can_id_mask(config.format);
          filter_mask = config.mask & can_id_mask(config.format);
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_List32>) {
          filter_id = config.id1 & can_id_mask(config.format);
          filter_mask = can_id_mask(config.format);
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_Mask16>) {
          filter_id = normalize_bx16_value(config.id1);
          filter_mask = normalize_bx16_value(config.mask1);
          return true;
        } else if constexpr (std::is_same_v<config_type, Bx_List16>) {
          filter_id = normalize_bx16_value(config.id1);
          filter_mask = 0x7FFU;
          return true;
        } else {
          return false;
        }
      },
      filter.config);
}

result apply_filter(const Bx_canId id, const Bx_filter& filter,
                    const bool enabled) noexcept {
  if (!enabled) {
    return clear_filter(id, filter.fifo);
  }

  uint32_t filter_id{0U};
  uint32_t filter_mask{0U};
  if (!translate_filter(filter, filter_id, filter_mask)) {
    return result::RECOVERABLE_ERROR;
  }

  return set_filter_on_backend(id, filter.fifo, filter_id, filter_mask);
}
}  // namespace

expected::expected<CanMessage, result> Bx_canRx::read(Bx_fifo fifo) noexcept {
  return const_cast<Bx_can&>(m_can).read(fifo);
}

expected::expected<CanMessage, result> Bx_canRx::try_read(Bx_fifo fifo) noexcept {
  return const_cast<Bx_can&>(m_can).try_read(fifo);
}

result Bx_canTx::write(const CanFrameView& message) noexcept {
  return const_cast<Bx_can&>(m_can).write(message);
}

result Bx_canTx::try_write(const CanFrameView& message) noexcept {
  return const_cast<Bx_can&>(m_can).try_write(message);
}

Bx_can::Bx_can(const Bx_canId id) noexcept : m_id(id) {
}

result Bx_can::start() noexcept {
  return result::OK;
}

result Bx_can::init() noexcept {
  const auto status = init_can(m_id);
  if (status != result::OK) {
    return status;
  }

  state(m_id).initialized = true;
  return result::OK;
}

result Bx_can::stop() noexcept {
  state(m_id).initialized = false;
  return result::OK;
}

expected::expected<CanMessage, result> Bx_can::read(Bx_fifo fifo) noexcept {
  CanMessage message{};
  const auto status = read_message(m_id, fifo, message);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return message;
}

expected::expected<CanMessage, result> Bx_can::try_read(Bx_fifo fifo) noexcept {
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

result Bx_can::write(const CanFrameView& message) noexcept {
  if (!message.is_valid() || message.len > k_can_classic_max_payload) {
    return result::UNRECOVERABLE_ERROR;
  }

  return write_message(m_id, message);
}

result Bx_can::try_write(const CanFrameView& message) noexcept {
  return write(message);
}

result Bx_can::set_priority(Bx_fifo fifo, uint8_t priority) {
  state(m_id).priorities[static_cast<std::size_t>(fifo_index(fifo))] = priority;
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::get_priority(Bx_fifo fifo) {
  return state(m_id).priorities[static_cast<std::size_t>(fifo_index(fifo))];
}

result Bx_can::set_interrupt(Bx_fifo fifo, bool on) {
  state(m_id).interrupts[static_cast<std::size_t>(fifo_index(fifo))] = on;
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::is_interrupt_on(Bx_fifo fifo) {
  return static_cast<uint8_t>(
      state(m_id).interrupts[static_cast<std::size_t>(fifo_index(fifo))]);
}

result Bx_can::set_error_priority(uint8_t priority) {
  state(m_id).error_priority = priority;
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::get_error_priority() {
  return state(m_id).error_priority;
}

result Bx_can::set_error_interrupt(bool on) {
  state(m_id).error_interrupt = on;
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::is_error_interrupt_on() {
  return static_cast<uint8_t>(state(m_id).error_interrupt);
}

result Bx_can::set_filter(Bx_filter filter, uint8_t id) {
  if (id >= k_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& controller_state = state(m_id);
  controller_state.filters[id] = filter;
  controller_state.filter_enabled[id] = true;
  return apply_filter(m_id, filter, true);
}

result Bx_can::enable_filter(uint8_t id) {
  if (id >= k_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& controller_state = state(m_id);
  if (!controller_state.filters[id].has_value()) {
    return result::RECOVERABLE_ERROR;
  }

  controller_state.filter_enabled[id] = true;
  return apply_filter(m_id, controller_state.filters[id].value(), true);
}

result Bx_can::disable_filter(uint8_t id) {
  if (id >= k_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  auto& controller_state = state(m_id);
  if (!controller_state.filters[id].has_value()) {
    return result::RECOVERABLE_ERROR;
  }

  controller_state.filter_enabled[id] = false;
  return apply_filter(m_id, controller_state.filters[id].value(), false);
}

expected::expected<bool, result> Bx_can::is_filter_enabled(uint8_t id) {
  if (id >= k_filter_slots) {
    return expected::unexpected(result::UNRECOVERABLE_ERROR);
  }

  return state(m_id).filter_enabled[id];
}

result Bx_canRx::set_priority(Bx_fifo fifo, uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_priority(fifo, priority);
}

expected::expected<uint8_t, result> Bx_canRx::get_priority(Bx_fifo fifo) {
  return const_cast<Bx_can&>(m_can).get_priority(fifo);
}

result Bx_canRx::set_interrupt(Bx_fifo fifo, bool on) {
  return const_cast<Bx_can&>(m_can).set_interrupt(fifo, on);
}

expected::expected<uint8_t, result> Bx_canRx::is_interrupt_on(Bx_fifo fifo) {
  return const_cast<Bx_can&>(m_can).is_interrupt_on(fifo);
}

result Bx_canRx::set_error_priority(uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Bx_canRx::get_error_priority() {
  return const_cast<Bx_can&>(m_can).get_error_priority();
}

result Bx_canRx::set_error_interrupt(bool on) {
  return const_cast<Bx_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Bx_canRx::is_error_interrupt_on() {
  return const_cast<Bx_can&>(m_can).is_error_interrupt_on();
}

result Bx_canRx::set_filter(Bx_filter filter, uint8_t id) {
  return const_cast<Bx_can&>(m_can).set_filter(filter, id);
}

result Bx_canRx::enable_filter(uint8_t id) {
  return const_cast<Bx_can&>(m_can).enable_filter(id);
}

result Bx_canRx::disable_filter(uint8_t id) {
  return const_cast<Bx_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> Bx_canRx::is_filter_enabled(uint8_t id) {
  return const_cast<Bx_can&>(m_can).is_filter_enabled(id);
}

result Bx_canTx::set_error_priority(uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Bx_canTx::get_error_priority() {
  return const_cast<Bx_can&>(m_can).get_error_priority();
}

result Bx_canTx::set_error_interrupt(bool on) {
  return const_cast<Bx_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Bx_canTx::is_error_interrupt_on() {
  return const_cast<Bx_can&>(m_can).is_error_interrupt_on();
}
}  // namespace ru::driver
