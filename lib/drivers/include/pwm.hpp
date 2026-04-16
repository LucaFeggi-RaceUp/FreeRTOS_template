#pragma once

#include <cstdint>

#include "common.hpp"
#include "opaque_pwm.hpp"
#include "pwm_id.hpp"

namespace ru::driver {

class Pwm {
 public:
  Pwm(PwmId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  result enable() noexcept;
  result disable() noexcept;
  result set_frequency(uint32_t frequency_hz) noexcept;
  result set_duty_cycle(uint16_t duty_cycle_permille) noexcept;
  expected::expected<uint16_t, result> get_duty_cycle() const noexcept;

  PwmId inline id() const noexcept { return m_id; }

 private:
  PwmId m_id;
  opaque_pwm m_opaque;
};

}  // namespace ru::driver
