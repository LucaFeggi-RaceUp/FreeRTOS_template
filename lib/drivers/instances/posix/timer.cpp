#include "timer.hpp"

using namespace ru::driver;

namespace ru::driver {
Timer::Timer(const TimerId id) noexcept : m_id(id) {
}

result Timer::start() noexcept {
  return opaque_timer::start();
}

result Timer::init() noexcept {
  return m_opaque.init();
}

result Timer::stop() noexcept {
  return m_opaque.stop();
}

expected::expected<TimerInstant, result> Timer::time_now() const noexcept {
  return m_opaque.time_now();
}
}  // namespace ru::driver
