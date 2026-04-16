#include "opaque_gpio.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
result opaque_gpio::init() const noexcept {
  if (m_is_output) {
    init_output_pin(m_p_port, m_pin);
    HAL_GPIO_WritePin(m_p_port, m_pin, inactive_state());
  } else {
    init_input_pin(m_p_port, m_pin);
  }
  return result::OK;
}

result opaque_gpio::stop() const noexcept {
  HAL_GPIO_DeInit(m_p_port, m_pin);
  return result::OK;
}

bool opaque_gpio::is_active() const noexcept {
  return HAL_GPIO_ReadPin(m_p_port, m_pin) == m_active_state;
}

bool opaque_gpio::is_high() const noexcept {
  return HAL_GPIO_ReadPin(m_p_port, m_pin) == GPIO_PIN_SET;
}

result opaque_gpio::set_level(const bool active) const noexcept {
  if (!m_is_output) {
    return result::RECOVERABLE_ERROR;
  }

  HAL_GPIO_WritePin(m_p_port, m_pin, active ? m_active_state : inactive_state());
  return result::OK;
}

result opaque_gpio::toggle() const noexcept {
  if (!m_is_output) {
    return result::RECOVERABLE_ERROR;
  }

  HAL_GPIO_TogglePin(m_p_port, m_pin);
  return result::OK;
}
}  // namespace ru::driver
