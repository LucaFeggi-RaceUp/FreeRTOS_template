#pragma once

#include "common.hpp"

namespace ru::driver {

#define SERIAL_LIST(X) X(USB)

DECLARE_ID_ENUM(Serial, SERIAL_LIST)

}  // namespace ru::driver
