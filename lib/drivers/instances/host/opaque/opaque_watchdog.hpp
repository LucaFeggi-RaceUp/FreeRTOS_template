#pragma once

#include <cstdint>

namespace ru::driver {
struct opaque_watchdog {
  uint32_t timeout_ms{0U};
  bool initialized{false};
};
}  // namespace ru::driver
