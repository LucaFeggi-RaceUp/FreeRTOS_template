#pragma once

#include <cstdint>

#include "common.hpp"
#include "opaque_watchdog.hpp"
#include "watchdog_id.hpp"

namespace ru::driver {

class Watchdog {
 public:
  Watchdog(WatchdogId id, uint32_t timeout_ms = 250U) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  result kick() noexcept;
  result refresh() noexcept;

  WatchdogId inline id() const noexcept { return m_id; }
  uint32_t inline timeout_ms() const noexcept { return m_timeout_ms; }

 private:
  WatchdogId m_id;
  uint32_t m_timeout_ms;
  struct opaque_watchdog m_opaque;
};

}  // namespace ru::driver
