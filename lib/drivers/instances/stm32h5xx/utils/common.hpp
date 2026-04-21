#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "common.hpp"

#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_adc_ex.h"
#include "stm32h5xx_hal_gpio_ex.h"
#include "stm32h5xx_hal_uart_ex.h"

namespace ru::driver {
template <typename Enum>
constexpr std::size_t driver_index(const Enum value) noexcept {
  return static_cast<std::size_t>(static_cast<std::underlying_type_t<Enum>>(value));
}

constexpr result from_hal_status(const HAL_StatusTypeDef status) noexcept {
  return status == HAL_OK ? result::OK : result::RECOVERABLE_ERROR;
}

constexpr uint32_t timeout_to_ms(const Timestamp timeout_uS) noexcept {
  if (timeout_uS == 0U) {
    return 0U;
  }

  constexpr uint64_t k_max_timeout_ms = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max());
  const auto timeout_ms = (timeout_uS + 999ULL) / 1000ULL;
  return static_cast<uint32_t>(timeout_ms > k_max_timeout_ms ? k_max_timeout_ms : timeout_ms);
}

inline void enable_gpio_clock(GPIO_TypeDef* const p_port) noexcept {
  if (p_port == GPIOA) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  } else if (p_port == GPIOB) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  } else if (p_port == GPIOC) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  } else if (p_port == GPIOD) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  } else if (p_port == GPIOE) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
}

inline void init_output_pin(GPIO_TypeDef* const p_port, const uint16_t pin) noexcept {
  enable_gpio_clock(p_port);

  GPIO_InitTypeDef init{};
  init.Pin = pin;
  init.Mode = GPIO_MODE_OUTPUT_PP;
  init.Pull = GPIO_NOPULL;
  init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(p_port, &init);
}

inline void init_input_pin(GPIO_TypeDef* const p_port, const uint16_t pin) noexcept {
  enable_gpio_clock(p_port);

  GPIO_InitTypeDef init{};
  init.Pin = pin;
  init.Mode = GPIO_MODE_INPUT;
  init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(p_port, &init);
}

inline void init_analog_pin(GPIO_TypeDef* const p_port, const uint16_t pin) noexcept {
  enable_gpio_clock(p_port);

  GPIO_InitTypeDef init{};
  init.Pin = pin;
  init.Mode = GPIO_MODE_ANALOG;
  init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(p_port, &init);
}

inline void init_alternate_pin(GPIO_TypeDef* const p_port, const uint16_t pin,
                               const uint32_t alternate) noexcept {
  enable_gpio_clock(p_port);

  GPIO_InitTypeDef init{};
  init.Pin = pin;
  init.Mode = GPIO_MODE_AF_PP;
  init.Pull = GPIO_PULLUP;
  init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  init.Alternate = alternate;
  HAL_GPIO_Init(p_port, &init);
}

inline void enable_serial_clock(USART_TypeDef* const p_instance) noexcept {
  if (p_instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
  } else if (p_instance == USART2) {
    __HAL_RCC_USART2_CLK_ENABLE();
  } else if (p_instance == USART3) {
    __HAL_RCC_USART3_CLK_ENABLE();
  } else if (p_instance == UART4) {
    __HAL_RCC_UART4_CLK_ENABLE();
  }
}

inline void enable_adc_clock(ADC_TypeDef* const p_instance) noexcept {
  (void)p_instance;
  __HAL_RCC_ADC_CLK_ENABLE();
}

inline void enable_fdcan_clock(FDCAN_GlobalTypeDef* const p_instance) noexcept {
  (void)p_instance;
  __HAL_RCC_FDCAN_CLK_ENABLE();
}

constexpr uint32_t fdcan_dlc_from_length(const uint8_t len) noexcept {
  switch (len) {
    case 0U:
      return FDCAN_DLC_BYTES_0;
    case 1U:
      return FDCAN_DLC_BYTES_1;
    case 2U:
      return FDCAN_DLC_BYTES_2;
    case 3U:
      return FDCAN_DLC_BYTES_3;
    case 4U:
      return FDCAN_DLC_BYTES_4;
    case 5U:
      return FDCAN_DLC_BYTES_5;
    case 6U:
      return FDCAN_DLC_BYTES_6;
    case 7U:
      return FDCAN_DLC_BYTES_7;
    case 8U:
      return FDCAN_DLC_BYTES_8;
    case 12U:
      return FDCAN_DLC_BYTES_12;
    case 16U:
      return FDCAN_DLC_BYTES_16;
    case 20U:
      return FDCAN_DLC_BYTES_20;
    case 24U:
      return FDCAN_DLC_BYTES_24;
    case 32U:
      return FDCAN_DLC_BYTES_32;
    case 48U:
      return FDCAN_DLC_BYTES_48;
    default:
      return FDCAN_DLC_BYTES_64;
  }
}

constexpr uint8_t fdcan_length_from_dlc(const uint32_t dlc) noexcept {
  switch (dlc) {
    case FDCAN_DLC_BYTES_0:
      return 0U;
    case FDCAN_DLC_BYTES_1:
      return 1U;
    case FDCAN_DLC_BYTES_2:
      return 2U;
    case FDCAN_DLC_BYTES_3:
      return 3U;
    case FDCAN_DLC_BYTES_4:
      return 4U;
    case FDCAN_DLC_BYTES_5:
      return 5U;
    case FDCAN_DLC_BYTES_6:
      return 6U;
    case FDCAN_DLC_BYTES_7:
      return 7U;
    case FDCAN_DLC_BYTES_8:
      return 8U;
    case FDCAN_DLC_BYTES_12:
      return 12U;
    case FDCAN_DLC_BYTES_16:
      return 16U;
    case FDCAN_DLC_BYTES_20:
      return 20U;
    case FDCAN_DLC_BYTES_24:
      return 24U;
    case FDCAN_DLC_BYTES_32:
      return 32U;
    case FDCAN_DLC_BYTES_48:
      return 48U;
    default:
      return 64U;
  }
}

inline void enable_tim_clock(TIM_TypeDef* const p_instance) noexcept {
  if (p_instance == TIM1) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  } else if (p_instance == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  } else if (p_instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  } else if (p_instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  } else if (p_instance == TIM8) {
    __HAL_RCC_TIM8_CLK_ENABLE();
  }
}
}  // namespace ru::driver
