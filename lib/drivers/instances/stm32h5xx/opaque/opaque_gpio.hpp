#pragma once

#include <cstdint>

#include "common.hpp"
#include "stm32h5xx_hal.h"

namespace ru::driver {
class Gpio;

struct opaque_gpio {
 public:
  constexpr opaque_gpio() noexcept = default;
  constexpr opaque_gpio(GPIO_TypeDef* const p_port, const uint16_t pin,
                        const GPIO_PinState active_state,
                        const bool is_output) noexcept
      : m_p_port(p_port),
        m_pin(pin),
        m_active_state(active_state),
        m_is_output(is_output) {}

 private:
  friend class Gpio;

  result init() const noexcept;
  result stop() const noexcept;
  bool is_active() const noexcept;
  bool is_high() const noexcept;
  result set_level(bool active) const noexcept;
  result toggle() const noexcept;
  constexpr bool active_high() const noexcept { return m_active_state == GPIO_PIN_SET; }
  constexpr GPIO_PinState inactive_state() const noexcept {
    return m_active_state == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET;
  }

  GPIO_TypeDef* m_p_port{nullptr};
  uint16_t m_pin{0U};
  GPIO_PinState m_active_state{GPIO_PIN_SET};
  bool m_is_output{true};
};
}  // namespace ru::driver
