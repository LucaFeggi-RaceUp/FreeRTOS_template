#pragma once

#include <cstdint>

#include "common.hpp"

namespace ru::driver {
struct opaque_pwm {
  using read_fn = result (*)(uint16_t& r_value) noexcept;
  using write_fn = result (*)(uint16_t duty_cycle_permille) noexcept;

  read_fn m_read{};
  write_fn m_write{};

  result read(uint16_t& r_value) const noexcept {
    return m_read != nullptr ? m_read(r_value) : result::RECOVERABLE_ERROR;
  }

  result write(const uint16_t duty_cycle_permille) const noexcept {
    return m_write != nullptr ? m_write(duty_cycle_permille)
                              : result::RECOVERABLE_ERROR;
  }
};
}  // namespace ru::driver
