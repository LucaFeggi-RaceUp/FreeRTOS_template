#pragma once

/*
 * Keep these X-macro maps aligned with config/tmpl/driver_ids/*.hpp.
 *
 * Maps intentionally stay empty until matching logical IDs are enabled.
 */

#define RU_POSIX_GPIO_MAP(X)

#define RU_POSIX_ADC_MAP(X)

#define RU_POSIX_PWM_MAP(X)

#define RU_POSIX_BX_CAN_MAP(X)                            \
  X(CAN_1, can0)                                          \
  X(CAN_2, can1)

#define RU_POSIX_SERIAL_MAP(X)                            \
  X(USB, RU_POSIX_SERIAL_DEVICE)

#define RU_POSIX_NV_MEMORY_MAP(X)

#define RU_POSIX_WATCHDOG_MAP(X)
