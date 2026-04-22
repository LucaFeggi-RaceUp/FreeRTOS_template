#pragma once

#include "common.hpp"

namespace ru::driver {

#define TIMER_LIST(X) X(TIMER_0)

DECLARE_ID_ENUM(Timer, TIMER_LIST)

}  // namespace ru::driver
