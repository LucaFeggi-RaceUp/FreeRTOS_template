#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "common.hpp"

namespace ru::driver {
class NvMemory;

struct opaque_nv_memory {
 public:
  opaque_nv_memory() noexcept = default;
  explicit opaque_nv_memory(std::string path, const uint32_t offset = 0U,
                            const uint32_t capacity = 0U) noexcept
      : m_path(std::move(path)), m_offset(offset), m_capacity(capacity) {}

 private:
  friend class NvMemory;

  result init() noexcept;
  result stop() noexcept;
  uint32_t capacity() const noexcept { return m_capacity; }
  result clear() noexcept;
  result read(uint32_t address, uint8_t* p_data, size_t len) noexcept;
  result write(uint32_t address, const uint8_t* p_data, size_t len) noexcept;
  result ensure_ready() noexcept;

  std::string m_path{};
  uint32_t m_offset{0U};
  uint32_t m_capacity{0U};
  int m_fd{-1};
};
}  // namespace ru::driver
