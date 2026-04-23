#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
struct opaque_nv_memory;

struct opaque_nv_memory_flash_region {
 public:
  constexpr opaque_nv_memory_flash_region() noexcept = default;

 private:
  friend struct opaque_nv_memory;

  result init() noexcept;
  result stop() noexcept;
  uint32_t capacity() const noexcept;
  result read(uint32_t addr, uint8_t* p_data, size_t len) const noexcept;
  result write(uint32_t addr, const uint8_t* p_data, size_t len) noexcept;
  result erase(uint32_t addr, size_t len) noexcept;
};
}  // namespace ru::driver
