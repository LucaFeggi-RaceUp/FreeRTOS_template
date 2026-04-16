#include "watchdog.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Watchdog::Watchdog(const WatchdogId id, const uint32_t timeout_ms) noexcept
    : m_id(id), m_timeout_ms(timeout_ms), m_opaque() {
  LOG("Watchdog " << toText(m_id) << " create timeout_ms=" << raw_value(timeout_ms));
}

result Watchdog::start() noexcept {
  LOG("Watchdog driver start");
  return result::OK;
}

result Watchdog::init() noexcept {
  if (m_timeout_ms == 0U) {
    return result::RECOVERABLE_ERROR;
  }

  LOG("Watchdog " << toText(m_id) << " init");
  m_opaque.timeout_ms = m_timeout_ms;
  m_opaque.initialized = true;
  return result::OK;
}

result Watchdog::stop() noexcept {
  LOG("Watchdog " << toText(m_id) << " stop");
  m_opaque.initialized = false;
  return result::OK;
}

result Watchdog::kick() noexcept {
  if (!m_opaque.initialized) {
    return result::RECOVERABLE_ERROR;
  }

  LOG("Watchdog " << toText(m_id) << " kick");
  return result::OK;
}

result Watchdog::refresh() noexcept {
  return kick();
}
}  // namespace ru::driver
