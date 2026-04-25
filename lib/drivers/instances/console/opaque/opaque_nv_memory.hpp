#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
class NvMemory;

struct opaque_nv_memory {
 public:
  constexpr opaque_nv_memory() noexcept = default;
  constexpr explicit opaque_nv_memory(const uint32_t capacity) noexcept
      : m_capacity(capacity) {}

 private:
  friend class NvMemory;

  result init() noexcept { return result::OK; }
  result stop() noexcept { return result::OK; }
  uint32_t capacity() const noexcept { return m_capacity; }
  result clear() noexcept { return result::OK; }

  result read(const uint32_t address, uint8_t* const p_data,
              const size_t len) const noexcept {
    if ((len != 0U && p_data == nullptr) ||
        (static_cast<uint64_t>(address) + static_cast<uint64_t>(len) >
         static_cast<uint64_t>(m_capacity))) {
      return result::RECOVERABLE_ERROR;
    }

    if (len == 0U) {
      return result::OK;
    }

    std::fill_n(p_data, len, static_cast<uint8_t>(0xFFU));
    return result::OK;
  }

  result write(const uint32_t address, const uint8_t* const p_data,
               const size_t len) noexcept {
    if ((len != 0U && p_data == nullptr) ||
        (static_cast<uint64_t>(address) + static_cast<uint64_t>(len) >
         static_cast<uint64_t>(m_capacity))) {
      return result::RECOVERABLE_ERROR;
    }

    return result::OK;
  }

  uint32_t m_capacity{0U};
};
}  // namespace ru::driver
