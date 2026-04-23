#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
struct opaque_nv_memory;

struct opaque_nv_memory_eeprom {
 public:
  constexpr opaque_nv_memory_eeprom() noexcept = default;
  constexpr opaque_nv_memory_eeprom(const uint16_t base_virtual_address,
                                    const uint32_t capacity) noexcept
      : m_base_virtual_address(base_virtual_address), m_capacity(capacity) {}

 private:
  friend struct opaque_nv_memory;

  result init() noexcept;
  result stop() noexcept;
  uint32_t capacity() const noexcept;
  result clear() noexcept;
  result read(uint32_t address, uint8_t* p_data, size_t len) const noexcept;
  result write(uint32_t address, const uint8_t* p_data, size_t len) noexcept;

  uint16_t m_base_virtual_address{0U};
  uint32_t m_capacity{0U};
};
}  // namespace ru::driver
