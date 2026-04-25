#include "watchdog.hpp"

using namespace ru::driver;

namespace ru::driver {
Watchdog::Watchdog(const WatchdogId id, const uint32_t timeout_ms) noexcept
    : m_id(id), m_timeout_ms(timeout_ms), m_opaque() {
}

result Watchdog::start() noexcept {
  return result::OK;
}

result Watchdog::init() noexcept {
  return m_opaque.init(m_timeout_ms);
}

result Watchdog::stop() noexcept {
  return m_opaque.stop();
}

result Watchdog::kick() noexcept {
  return m_opaque.kick();
}

result Watchdog::refresh() noexcept {
  return kick();
}
}  // namespace ru::driver
