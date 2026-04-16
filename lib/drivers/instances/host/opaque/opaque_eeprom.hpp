#pragma once

#include <array>
#include <cstdint>

namespace ru::driver {
struct opaque_eeprom {
  opaque_eeprom() noexcept { storage.fill(0xFFU); }

  std::array<uint8_t, 32U> storage{};
  bool initialized{false};
};
}  // namespace ru::driver
