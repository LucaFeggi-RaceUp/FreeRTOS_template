#pragma once

#include "common.hpp"

namespace ru::driver {

#define TIMER_LIST(X) X(TIMER)

DECLARE_ID_ENUM(Timer, TIMER_LIST)

}  // namespace ru::driver
