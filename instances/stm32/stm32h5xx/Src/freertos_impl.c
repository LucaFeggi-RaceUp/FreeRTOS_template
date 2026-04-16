/***
* CHATGPT GENERATED CODE !!!
*
* This code implements callbacks, hooks, and static memory allocation
* functions required by FreeRTOS for an STM32H5xx microcontroller.
*/

#include "FreeRTOS.h"
#include "task.h"
#include "stm32h5xx_hal.h"

static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

// Assert Callback function
extern void vAssertCalled(const char *const pcFileName, unsigned long ulLine)
{
    taskDISABLE_INTERRUPTS();
    // You can log file and line here if you have UART
    for(;;); // halt the system
}


// Idle task - for static allocation
static StaticTask_t xIdleTaskTCB;
static StackType_t  xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = xIdleStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

// Timer task - for static allocation
static StaticTask_t xTimerTaskTCB;
static StackType_t  xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = xTimerStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

// Tick hook function for HAL tick increment
void vApplicationTickHook(void) {
    HAL_IncTick();
}
