#pragma once

#include "common.hpp"

namespace ru::driver {
struct opaque_gpio {
  using read_fn = result (*)(bool& r_value) noexcept;
  using write_fn = result (*)(bool value) noexcept;

  read_fn m_read{};
  write_fn m_write{};
  bool m_active_high{true};
  bool m_is_output{true};

  constexpr bool active_high() const noexcept { return m_active_high; }

  result read(bool& r_value) const noexcept {
    return m_read != nullptr ? m_read(r_value) : result::RECOVERABLE_ERROR;
  }

  result write(const bool value) const noexcept {
    return m_write != nullptr ? m_write(value) : result::RECOVERABLE_ERROR;
  }
};
}  // namespace ru::driver
