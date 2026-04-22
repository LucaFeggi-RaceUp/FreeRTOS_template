#pragma once

#ifndef FREERTOS_CPU_CLOCK_HZ
#error "FREERTOS_CPU_CLOCK_HZ must be defined via CMake"
#endif
#ifndef FREERTOS_TICK_RATE_HZ
#error "FREERTOS_TICK_RATE_HZ must be defined via CMake"
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void vAssertCalled(const char *const pcFileName, unsigned long ulLine);
#ifdef __cplusplus
}
#endif

#ifdef DEBUG
  #define configASSERT(x) if ((x) == 0) vAssertCalled(__FILE__, __LINE__)
#else
  #define configASSERT(x) ((void)0)
#endif

// OS
#define configCPU_CLOCK_HZ ((unsigned long)FREERTOS_CPU_CLOCK_HZ)
#define configTICK_RATE_HZ ((unsigned long)FREERTOS_TICK_RATE_HZ)
#define configUSE_PREEMPTION 1
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif
#define configIDLE_SHOULD_YIELD 1
#define configMAX_PRIORITIES 8
#define configUSE_TICKLESS_IDLE 0
#define configUSE_TIME_SLICING 1
#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES 5

#ifdef DEBUG
#else
#endif

// RAM and OPT
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configMINIMAL_STACK_SIZE (1024 * 1)
#ifndef FREERTOS_HEAP_BYTES
#define FREERTOS_HEAP_BYTES (1024 * 2)
#endif
#define configTOTAL_HEAP_SIZE FREERTOS_HEAP_BYTES
#define configAPPLICATION_ALLOCATED_HEAP 0
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0
#define configMAX_TASK_NAME_LEN 30
#define configUSE_16_BIT_TICKS 0
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 3
#define configUSE_MINI_LIST_ITEM 0
#define configSTACK_DEPTH_TYPE uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t
#define configHEAP_CLEAR_MEMORY_ON_FREE 0
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

#ifdef DEBUG
#define configQUEUE_REGISTRY_SIZE 10
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#else
#define configQUEUE_REGISTRY_SIZE 0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0
#endif

// DS
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_QUEUE_SETS 1
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY 3

#ifdef DEBUG
#else
#endif

// HOOKS
#define configUSE_IDLE_HOOK 0
#ifndef configUSE_TICK_HOOK
  #define configUSE_TICK_HOOK 0
#endif
#define configCHECK_FOR_STACK_OVERFLOW 0
#define configUSE_MALLOC_FAILED_HOOK 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0
#define configUSE_SB_COMPLETED_CALLBACK 0

#ifdef DEBUG
#else
#endif

// LEGACY
#define configUSE_ALTERNATIVE_API 0 /* Deprecated! */
#define configUSE_NEWLIB_REENTRANT 0
#define configENABLE_BACKWARD_COMPATIBILITY 0 // pre 8.0.0

#ifdef DEBUG
#else
#endif

// API INCLUDES
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskCleanUpResources 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTimerPendFunctionCall 1
#define INCLUDE_eTaskGetState 1
#define INCLUDE_xTaskAbortDelay 1
#define INCLUDE_xTaskGetTaskHandle 1
#define INCLUDE_xTaskGetHandle 1
#define INCLUDE_xSemaphoreGetMutexHolder 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
