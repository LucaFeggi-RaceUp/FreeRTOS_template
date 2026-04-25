#include "pwm.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Pwm::Pwm(const PwmId id) noexcept : m_id(id) {
  LOG("Pwm " << toText(m_id) << " create");
}

result Pwm::start() noexcept {
  LOG("Pwm driver start");
  return result::OK;
}

result Pwm::init() noexcept {
  LOG("Pwm " << toText(m_id) << " init");
  return result::OK;
}

result Pwm::stop() noexcept {
  LOG("Pwm " << toText(m_id) << " stop");
  return result::OK;
}

result Pwm::enable() noexcept {
  LOG("Pwm " << toText(m_id) << " enable");
  return result::OK;
}

result Pwm::disable() noexcept {
  LOG("Pwm " << toText(m_id) << " disable");
  return result::OK;
}

result Pwm::set_frequency(const uint32_t frequency_hz) noexcept {
  LOG("Pwm " << toText(m_id) << " set_frequency");
  return result::OK;
}

result Pwm::set_duty_cycle(const uint16_t duty_cycle_permille) noexcept {
  LOG("Pwm " << toText(m_id) << "set_duty");
  return result::OK;
}

expected::expected<uint16_t, result> Pwm::get_duty_cycle() const noexcept {
  return 0;
}
}  // namespace ru::driver
