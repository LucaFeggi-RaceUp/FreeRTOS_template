#pragma once

#include "common.hpp"

namespace ru::driver {

#define ADC_LIST(X) X(POT_0)

DECLARE_ID_ENUM(Adc, ADC_LIST)

}  // namespace ru::driver
