#pragma once

#include <cstdint>

#include "common.hpp"

namespace ru::driver {
struct opaque_adc {
  using read_fn = result (*)(uint16_t& r_value) noexcept;

  read_fn m_read{};

  result read(uint16_t& r_value) const noexcept {
    return m_read != nullptr ? m_read(r_value) : result::UNRECOVERABLE_ERROR;
  }

};
}  // namespace ru::driver
