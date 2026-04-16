#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
class FlashMemory;
struct opaque_eeprom;

struct opaque_flash_memory {
 public:
  constexpr opaque_flash_memory() noexcept = default;

 private:
  friend class FlashMemory;
  friend struct opaque_eeprom;

  result init() noexcept;
  result stop() noexcept;
  result read(uint32_t addr, uint8_t* p_data, size_t len) const noexcept;
  result write(uint32_t addr, const uint8_t* p_data, size_t len) noexcept;
  result erase(uint32_t addr, size_t len) noexcept;
  result erase_all() noexcept;
};
}  // namespace ru::driver
