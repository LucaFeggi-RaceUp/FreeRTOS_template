#pragma once

#include "common.hpp"

namespace ru::driver {

#define M_CAN_LIST(X) X(CAN_2)
#define BX_CAN_LIST(X) X(CAN_2)
#define FLEX_CAN_LIST(X) X(CAN_0)
#define MULTI_CAN_LIST(X) X(CAN_0)

DECLARE_ID_ENUM(M_can, M_CAN_LIST)
DECLARE_ID_ENUM(Bx_can, BX_CAN_LIST)
DECLARE_ID_ENUM(Flex_can, FLEX_CAN_LIST)
DECLARE_ID_ENUM(Multi_can, MULTI_CAN_LIST)

}  // namespace ru::driver
