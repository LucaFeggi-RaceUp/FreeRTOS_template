#include <cstddef>
#include <cstdint>

#include "FreeRTOS.h"
#include "serial.hpp"
#include "task.h"

namespace {

constexpr uint32_t k_app_task_stack_depth = 512U;
constexpr UBaseType_t k_app_task_priority = tskIDLE_PRIORITY + 1U;
constexpr TickType_t k_app_task_period = pdMS_TO_TICKS(1000U);
constexpr Timestamp k_serial_timeout_us = 100'000U;

StaticTask_t g_app_task_tcb;
StackType_t g_app_task_stack[k_app_task_stack_depth];
TaskHandle_t g_app_task_handle = nullptr;

void write_line(ru::driver::Serial& serial, const char* const text) noexcept {
  if (text == nullptr) {
    return;
  }

  std::size_t length = 0U;
  while (text[length] != '\0') {
    ++length;
  }

  (void)serial.write(reinterpret_cast<const uint8_t*>(text), length,
                     k_serial_timeout_us);
}

void app_task(void* arg) {
  (void)arg;

  ru::driver::Serial serial;
  (void)serial.init();

  TickType_t last_wake_time = xTaskGetTickCount();
  for (;;) {
    write_line(serial, "template app: alive\r\n");
    vTaskDelayUntil(&last_wake_time, k_app_task_period);
  }
}

}  // namespace

void app_start(void) {
  if (g_app_task_handle != nullptr) {
    return;
  }

  g_app_task_handle =
      xTaskCreateStatic(app_task, "app", k_app_task_stack_depth, nullptr,
                        k_app_task_priority, g_app_task_stack, &g_app_task_tcb);
  configASSERT(g_app_task_handle != nullptr);
}
