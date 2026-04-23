#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"
#include "nv_memory_id.hpp"
#include "opaque_nv_memory.hpp"

namespace ru::driver {

class NvMemory {
 public:
  explicit NvMemory(NvMemoryId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  uint32_t capacity() const noexcept;
  result clear() noexcept;
  result read(uint32_t address, uint8_t* data, size_t len) noexcept;
  expected::expected<uint8_t, result> read(uint32_t address) noexcept;
  result write(uint32_t address, const uint8_t* data, size_t len) noexcept;
  result write(uint32_t address, uint8_t value) noexcept;

  NvMemoryId id() const noexcept { return m_id; }

 private:
  NvMemoryId m_id;
  struct opaque_nv_memory m_opaque;
};

}  // namespace ru::driver
