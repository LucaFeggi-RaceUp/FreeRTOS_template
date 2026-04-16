#include "gpio.hpp"

#include "common.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Gpio::Gpio(const GpioId id) noexcept : m_id(id) {
  LOG("Gpio " << toText(m_id) << " create");
}

result Gpio::start() noexcept {
  LOG("Gpio driver start");
  return result::OK;
}

result Gpio::init() noexcept {
  LOG("Gpio " << toText(m_id) << " init");
  return result::OK;
}

result Gpio::stop() noexcept {
  LOG("Gpio " << toText(m_id) << " stop");
  return result::OK;
}

GpioValue Gpio::active_value() const noexcept { return GpioValue::HIGH; }

GpioPolarity Gpio::polarity() const noexcept {
  return GpioPolarity::ACTIVE_HIGH;
}

expected::expected<bool, result> Gpio::is_active() const noexcept {
  return true;
}

expected::expected<bool, result> Gpio::is_inactive() const noexcept {
  return false;
}

expected::expected<bool, result> Gpio::is_high() const noexcept { return true; }

expected::expected<bool, result> Gpio::is_low() const noexcept { return false; }

result Gpio::set_active() noexcept {
  LOG("Gpio " << toText(m_id) << " set active");
  return result::OK;
}

result Gpio::set_inactive() noexcept {
  LOG("Gpio " << toText(m_id) << " set inactive");
  return result::OK;
}

result Gpio::set_level(const bool active) noexcept {
  LOG("Gpio " << toText(m_id) << " set_level");
  return result::OK;
}

result Gpio::toggle() noexcept {
  LOG("Gpio " << toText(m_id) << " toggle");
  return result::OK;
}

}  // namespace ru::driver
