#pragma once

#include "common.hpp"

namespace ru::driver {

#define FLASH_LIST(X) X(FLASH_1)

DECLARE_ID_ENUM(FlashMemory, FLASH_LIST)

}  // namespace ru::driver
