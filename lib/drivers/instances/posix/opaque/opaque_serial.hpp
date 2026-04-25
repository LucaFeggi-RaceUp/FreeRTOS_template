#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "common.hpp"

namespace ru::driver {
class Serial;

struct opaque_serial {
 public:
  constexpr opaque_serial() noexcept = default;
  constexpr explicit opaque_serial(const std::string_view device_path) noexcept
      : m_device_path(device_path) {}

 private:
  friend class Serial;

  result init() noexcept;
  result stop() noexcept;
  result write(const uint8_t* p_data, size_t len, Timestamp timeout_uS) noexcept;
  result read(uint8_t* p_data, size_t len, Timestamp timeout_uS) noexcept;

  std::string_view m_device_path{};
  int m_fd{-1};
};
}  // namespace ru::driver
