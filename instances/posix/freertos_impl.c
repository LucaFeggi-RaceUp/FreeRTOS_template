#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

void vAssertCalled(const char *const pcFileName, unsigned long ulLine) {
  volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

  (void)ulLine;
  (void)pcFileName;

  taskENTER_CRITICAL();
  {
    printf("Assert failed: %s at line %ld\n", pcFileName, ulLine);
    while (ulSetToNonZeroInDebuggerToContinue == 0) {
      __asm volatile("NOP");
      __asm volatile("NOP");
    }
  }
  taskEXIT_CRITICAL();
}

void vLoggingPrintf(const char *pcFormat, ...) {
  va_list arg;

  va_start(arg, pcFormat);
  vprintf(pcFormat, arg);
  va_end(arg);
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  static StaticTask_t xTimerTaskTCB;
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *taskName) {
  for (;;) {
  }
}

#undef traceMOVED_TASK_TO_DELAYED_LIST
#define traceMOVED_TASK_TO_DELAYED_LIST()                                      \
  printf("%ld\n", pxCurrentTCB->uxTCBNumber);
