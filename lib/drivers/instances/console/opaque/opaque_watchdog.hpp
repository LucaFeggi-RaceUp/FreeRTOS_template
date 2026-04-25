#pragma once

#include <cstdint>

#include "common.hpp"

namespace ru::driver {
class Watchdog;

struct opaque_watchdog {
 private:
  friend class Watchdog;

  result init(const uint32_t timeout_ms) noexcept {
    if (timeout_ms == 0U) {
      return result::RECOVERABLE_ERROR;
    }

    m_timeout_ms = timeout_ms;
    m_initialized = true;
    return result::OK;
  }

  result stop() noexcept {
    m_initialized = false;
    return result::OK;
  }

  result kick() noexcept {
    return m_initialized ? result::OK : result::RECOVERABLE_ERROR;
  }

  uint32_t m_timeout_ms{0U};
  bool m_initialized{false};
};
}  // namespace ru::driver
