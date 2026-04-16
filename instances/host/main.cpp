#include "FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"

#include "common.hpp"
#include "serial.hpp"
#include "timer.hpp"

using namespace ru::driver;

void app_start(void);

int main(void) {
  TRY_WHILE_RECOVERABLE(Common::start());
  TRY_WHILE_RECOVERABLE(Timer::start());
  TRY_WHILE_RECOVERABLE(Serial::start());

  app_start();
  vTaskStartScheduler();

  for (;;) {
  }
}
