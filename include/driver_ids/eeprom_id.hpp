#pragma once

#include "common.hpp"

namespace ru::driver {

#define EEPROM_LIST(X) X(EEPROM_0)

DECLARE_ID_ENUM(Eeprom, EEPROM_LIST)

}  // namespace ru::driver
