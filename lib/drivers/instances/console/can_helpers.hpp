#pragma once

#include <array>
#include <optional>

#include "can/can.hpp"

namespace ru::driver {
inline CanMessage make_dummy_can_message() noexcept {
  constexpr std::array<uint8_t, 1U> k_payload{{0U}};

  ClassicStdCanMessage frame{};
  (void)frame.assign(0U, CanFrameFormat::Classic, k_payload.data(),
                     static_cast<uint8_t>(k_payload.size()));
  return CanMessage{frame};
}

inline CanMessageTs make_dummy_can_message_ts() noexcept {
  return CanMessageTs{make_dummy_can_message(), std::optional<uint8_t>{}, 0U};
}
}  // namespace ru::driver
