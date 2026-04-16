#include "opaque_timer.hpp"

#include "stm32h5xx_hal.h"

using namespace ru::driver;

namespace ru::driver {
result opaque_timer::init() const noexcept {
  return result::OK;
}

result opaque_timer::stop() const noexcept {
  return result::OK;
}

Timestamp opaque_timer::time_now() const noexcept {
  return static_cast<Timestamp>(HAL_GetTick()) * 1000ULL;
}
}  // namespace ru::driver
