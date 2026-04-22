#pragma once

#include "common.hpp"

namespace ru::driver {

#define WATCHDOG_LIST(X) X(IWDG_0)

DECLARE_ID_ENUM(Watchdog, WATCHDOG_LIST)

}  // namespace ru::driver
