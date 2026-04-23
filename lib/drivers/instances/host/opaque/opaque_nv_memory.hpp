#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
class NvMemory;

struct opaque_nv_memory {
 public:
  explicit opaque_nv_memory(uint32_t capacity = k_storage_capacity) noexcept;

 private:
  friend class NvMemory;

  result init() noexcept;
  result stop() noexcept;
  uint32_t capacity() const noexcept;
  result clear() noexcept;
  result read(uint32_t address, uint8_t* p_data, size_t len) const noexcept;
  result write(uint32_t address, const uint8_t* p_data, size_t len) noexcept;

  static constexpr uint32_t k_storage_capacity{256U};

  std::array<uint8_t, k_storage_capacity> m_storage{};
  uint32_t m_capacity{k_storage_capacity};
  bool m_initialized{false};
};
}  // namespace ru::driver
