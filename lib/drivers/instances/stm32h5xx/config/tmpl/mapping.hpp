#pragma once

#include "stm32h5xx_hal.h"

#define RU_STM32H5XX_GPIO_MAP(X)                                                        \

#define RU_STM32H5XX_ADC_MAP(X)                                                         \

#define RU_STM32H5XX_PWM_MAP(X)                                                         \

#define RU_STM32H5XX_TIMER_MAP(X)                                                       \
  X(TIMER_0, TIM2)

#define RU_STM32H5XX_SERIAL_MAP(X)                                                      \
  X(USB, USART1, GPIOA, GPIO_PIN_11, GPIOA, GPIO_PIN_10, GPIO_AF7_USART1)

#define RU_STM32H5XX_M_CAN_MAP(X)                                                       \
  X(CAN_1, FDCAN1, GPIOD, GPIO_PIN_0, GPIOD, GPIO_PIN_1, GPIO_AF9_FDCAN1)                \
  X(CAN_2, FDCAN2, GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_13, GPIO_AF9_FDCAN2)

#define RU_STM32H5XX_BX_CAN_MAP(X)                                                      \
  X(CAN_1, FDCAN1, GPIOD, GPIO_PIN_0, GPIOD, GPIO_PIN_1, GPIO_AF9_FDCAN1)                \
  X(CAN_2, FDCAN2, GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_13, GPIO_AF9_FDCAN2)

#define RU_STM32H5XX_NV_MEMORY_MAP(X)                                                   \
  X(BOOT_COUNTER, EmulatedEeprom, 1U, 32U)                                              \
  X(SETTINGS, FlashRegion, 0U, 128U)

#define RU_STM32H5XX_WATCHDOG_MAP(X)                                                    \
  X(IWDG_0)
