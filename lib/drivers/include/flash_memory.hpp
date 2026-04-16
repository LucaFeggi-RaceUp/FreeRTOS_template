#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"
#include "flash_memory_id.hpp"
#include "opaque_flash_memory.hpp"

namespace ru::driver {

class FlashMemory {
 public:
  FlashMemory(FlashMemoryId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  result read(uint32_t addr, uint8_t* data, size_t len) noexcept;
  result write(uint32_t addr, const uint8_t* data, size_t len) noexcept;
  result erase(uint32_t addr, size_t len) noexcept;
  result erase_all() noexcept;

  FlashMemoryId inline id() const noexcept { return m_id; }

 private:
  FlashMemoryId m_id;
  struct opaque_flash_memory m_opaque;
};

}  // namespace ru::driver
