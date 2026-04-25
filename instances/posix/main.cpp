// IWYU pragma: begin_exports
#include <cstdio>
#include "FreeRTOS.h"
// IWYU pragma: end_exports
#include "task.h"

#include "common.hpp"
#include "serial.hpp"

using namespace ru::driver;

void app_start(void);

int main(void) {
  TRY_WHILE_RECOVERABLE(Common::start());
  TRY_WHILE_RECOVERABLE(Serial::start());

  app_start();

  vTaskStartScheduler();

  for (;;) {
  }
}
