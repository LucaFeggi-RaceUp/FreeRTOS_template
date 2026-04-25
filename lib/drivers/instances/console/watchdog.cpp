#include "watchdog.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Watchdog::Watchdog(const WatchdogId id, const uint32_t timeout_ms) noexcept
    : m_id(id), m_timeout_ms(timeout_ms), m_opaque() {
  LOG("Watchdog " << toText(m_id) << " create timeout_ms=" << raw_value(m_timeout_ms));
}

result Watchdog::start() noexcept {
  LOG("Watchdog driver start");
  return result::OK;
}

result Watchdog::init() noexcept {
  LOG("Watchdog " << toText(m_id) << " init");
  return m_opaque.init(m_timeout_ms);
}

result Watchdog::stop() noexcept {
  LOG("Watchdog " << toText(m_id) << " stop");
  return m_opaque.stop();
}

result Watchdog::kick() noexcept {
  LOG("Watchdog " << toText(m_id) << " kick");
  return m_opaque.kick();
}

result Watchdog::refresh() noexcept {
  return kick();
}
}  // namespace ru::driver
