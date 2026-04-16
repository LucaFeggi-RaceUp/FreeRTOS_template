#pragma once

#include <optional>

#include "can/can.hpp"

namespace ru::driver {
inline CanMessage make_dummy_can_message() noexcept {
  CanMessage message{};
  message.id = 0U;
  message.len = 1U;
  message.full_word = 0U;
  return message;
}

inline CanMessageTs make_dummy_can_message_ts() noexcept {
  return CanMessageTs{make_dummy_can_message(), std::optional<uint8_t>{}, 0U};
}
}  // namespace ru::driver
