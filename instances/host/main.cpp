#include "FreeRTOS.h"
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

  for (;;) {
  }
}
