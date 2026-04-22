#pragma once

#include "common.hpp"

namespace ru::driver {

#define GPIO_LIST(X) X(LED_E3)

DECLARE_ID_ENUM(Gpio, GPIO_LIST)

}  // namespace ru::driver
