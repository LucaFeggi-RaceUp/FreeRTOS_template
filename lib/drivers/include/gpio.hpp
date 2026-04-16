#pragma once

#include <cstdint>

#include "common.hpp"
#include "gpio_id.hpp"
#include "opaque_gpio.hpp"

namespace ru::driver {

enum class GpioValue : uint8_t { LOW, HIGH };
enum class GpioPolarity : uint8_t { ACTIVE_LOW, ACTIVE_HIGH };

class Gpio {
 public:
  Gpio(GpioId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  GpioValue active_value() const noexcept;
  GpioPolarity polarity() const noexcept;
  expected::expected<bool, result> is_active() const noexcept;
  expected::expected<bool, result> is_inactive() const noexcept;
  expected::expected<bool, result> is_high() const noexcept;
  expected::expected<bool, result> is_low() const noexcept;
  result set_active() noexcept;
  result set_inactive() noexcept;
  result set_level(const bool active) noexcept;
  result toggle() noexcept;

  GpioId inline id() const noexcept { return m_id; }

 private:
  GpioId m_id;
  struct opaque_gpio m_opaque;
};

}  // namespace ru::driver
