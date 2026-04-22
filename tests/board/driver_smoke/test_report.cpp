#include "smoke_test.hpp"

#include <cstdio>
#include <cstring>

#include "task.h"

namespace smoke {
namespace {
constexpr uint32_t k_report_task_stack_depth = 1024U;
constexpr UBaseType_t k_report_task_priority = tskIDLE_PRIORITY + 2U;
constexpr TickType_t k_status_period = pdMS_TO_TICKS(1000U);

StaticTask_t g_report_task_tcb;
StackType_t g_report_task_stack[k_report_task_stack_depth];
TaskHandle_t g_report_task_handle = nullptr;

void report_task(void* arg) {
  auto& context = *static_cast<SmokeContext*>(arg);
  TickType_t last_status = xTaskGetTickCount();

  for (;;) {
    AdcSample sample{};
    if (context.adc_queue != nullptr &&
        xQueueReceive(context.adc_queue, &sample, pdMS_TO_TICKS(20U)) == pdPASS) {
      char line[96]{};
      std::snprintf(line, sizeof(line), "adc=%u sample=%lu tick_ms=%lu errors=0x%08lx\r\n",
                    static_cast<unsigned>(sample.value),
                    static_cast<unsigned long>(sample.counter),
                    static_cast<unsigned long>(sample.tick_ms),
                    static_cast<unsigned long>(error_bits(context)));
      usb_line(context, line);
      can_report_adc(context, sample);
    }

    can_service_rx(context);

    const TickType_t now = xTaskGetTickCount();
    if ((now - last_status) >= k_status_period) {
      last_status = now;
      char line[128]{};
      std::snprintf(line, sizeof(line),
                    "status errors=0x%08lx q=%lu timer=%lu eeprom_boot=%lu\r\n",
                    static_cast<unsigned long>(error_bits(context)),
                    static_cast<unsigned long>(context.freertos_queue_counter),
                    static_cast<unsigned long>(context.freertos_timer_counter),
                    static_cast<unsigned long>(context.eeprom_summary.boot_counter));
      usb_line(context, line);
      can_report_heartbeat(context);
    }
  }
}
}  // namespace

void set_error(SmokeContext& context, const uint32_t bit) noexcept {
  taskENTER_CRITICAL();
  context.errors |= bit;
  taskEXIT_CRITICAL();
}

uint32_t error_bits(SmokeContext& context) noexcept {
  taskENTER_CRITICAL();
  const uint32_t errors = context.errors;
  taskEXIT_CRITICAL();
  return errors;
}

void mark_result(SmokeContext& context, const ru::driver::result status,
                 const uint32_t bit) noexcept {
  if (status != ru::driver::result::OK) {
    set_error(context, bit);
  }
}

void usb_line(SmokeContext& context, const char* const text) noexcept {
  if (!context.usb_ready || text == nullptr) {
    return;
  }

  (void)context.usb.write(reinterpret_cast<const uint8_t*>(text),
                          std::strlen(text), k_usb_timeout_us);
}

void start_report_smoke(SmokeContext& context) noexcept {
  if (g_report_task_handle != nullptr) {
    return;
  }

  g_report_task_handle =
      xTaskCreateStatic(report_task, "smoke_report", k_report_task_stack_depth,
                        &context, k_report_task_priority, g_report_task_stack,
                        &g_report_task_tcb);
  if (g_report_task_handle == nullptr) {
    set_error(context, k_error_task_create);
  }
}

}  // namespace smoke
