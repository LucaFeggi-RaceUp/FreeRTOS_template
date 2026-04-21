#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "clock.hpp"
#include "common.hpp"
#include "serial.hpp"

using namespace ru::driver;

void app_start(void);

int main(void) {
  TRY_WHILE_RECOVERABLE(Common::start());
  TRY_WHILE_RECOVERABLE(Clock::start());
  TRY_WHILE_RECOVERABLE(Serial::start());

  app_start();

  vTaskStartScheduler();
  while (1) {
  }
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  (void)file;
  (void)line;

  Error_Handler();
}
#endif /* USE_FULL_ASSERT */
