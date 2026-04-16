#include "common.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {

result configure_system_clock() noexcept {
  RCC_OscInitTypeDef osc_init{};
  RCC_ClkInitTypeDef clk_init{};
  RCC_CRSInitTypeDef crs_init{};

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
  osc_init.HSIState = RCC_HSI_ON;
  osc_init.HSIDiv = RCC_HSI_DIV1;
  osc_init.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc_init.HSI48State = RCC_HSI48_ON;
  osc_init.PLL.PLLState = RCC_PLL_ON;
  osc_init.PLL.PLLSource = RCC_PLL1_SOURCE_HSI;
  osc_init.PLL.PLLM = 16;
  osc_init.PLL.PLLN = 125;
  osc_init.PLL.PLLP = 2;
  osc_init.PLL.PLLQ = 2;
  osc_init.PLL.PLLR = 2;
  osc_init.PLL.PLLRGE = RCC_PLL1_VCIRANGE_2;
  osc_init.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  osc_init.PLL.PLLFRACN = 0;

  if (HAL_RCC_OscConfig(&osc_init) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  clk_init.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                       RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
  clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk_init.APB1CLKDivider = RCC_HCLK_DIV1;
  clk_init.APB2CLKDivider = RCC_HCLK_DIV1;
  clk_init.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clk_init, FLASH_LATENCY_5) != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  __HAL_RCC_CRS_CLK_ENABLE();

  crs_init.Prescaler = RCC_CRS_SYNC_DIV1;
  crs_init.Source = RCC_CRS_SYNC_SOURCE_USB;
  crs_init.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  crs_init.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000U, 1000U);
  crs_init.ErrorLimitValue = 34U;
  crs_init.HSI48CalibrationValue = 32U;
  HAL_RCCEx_CRSConfig(&crs_init);

  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
  return result::OK;
}

}  // namespace

result opaque_common::start() const noexcept {
  const auto hal_status = HAL_Init();
  if (hal_status != HAL_OK) {
    return from_hal_status(hal_status);
  }

  return configure_system_clock();
}
}  // namespace ru::driver
