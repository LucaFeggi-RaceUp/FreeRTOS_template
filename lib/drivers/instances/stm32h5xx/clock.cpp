#include "clock.hpp"

#include "stm32h5xx_hal.h"

using namespace ru::driver;

namespace ru::driver {
result Clock::start() noexcept {
  return result::OK;
}

Timestamp Clock::now_us() noexcept {
  return static_cast<Timestamp>(HAL_GetTick()) * 1000ULL;
}
}  // namespace ru::driver
