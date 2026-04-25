#include "timer.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {

Timer::Timer(const TimerId id) noexcept : m_id(id) {
  LOG("Timer " << toText(m_id) << " create");
}

result Timer::start() noexcept {
  LOG("Timer driver start");
  return result::OK;
}

result Timer::init() noexcept {
  LOG("Timer " << toText(m_id) << " init");
  return result::OK;
}

result Timer::stop() noexcept {
  LOG("Timer " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<TimerInstant, result> Timer::time_now() const noexcept {
  return static_cast<TimerInstant>(0U);
}
}  // namespace ru::driver
