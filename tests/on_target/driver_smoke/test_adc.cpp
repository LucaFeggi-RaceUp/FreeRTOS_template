#include "smoke_test.hpp"

#include "task.h"

namespace smoke {
namespace {
constexpr uint32_t k_adc_task_stack_depth = 512U;
constexpr UBaseType_t k_adc_task_priority = tskIDLE_PRIORITY + 2U;
constexpr UBaseType_t k_adc_queue_length = 8U;

StaticTask_t g_adc_task_tcb;
StackType_t g_adc_task_stack[k_adc_task_stack_depth];
TaskHandle_t g_adc_task_handle = nullptr;
StaticQueue_t g_adc_queue_control;
uint8_t g_adc_queue_storage[k_adc_queue_length * sizeof(AdcSample)];

uint32_t ticks_to_ms(const TickType_t ticks) noexcept {
  return static_cast<uint32_t>(ticks) * static_cast<uint32_t>(portTICK_PERIOD_MS);
}

void adc_task(void* arg) {
  auto& context = *static_cast<SmokeContext*>(arg);
  uint32_t counter = 0U;
  TickType_t last_wake_time = xTaskGetTickCount();

  for (;;) {
    const auto value = context.pot.read();
    if (value.has_value()) {
      const TickType_t now = xTaskGetTickCount();
      const AdcSample sample{value.value(), counter++, ticks_to_ms(now)};
      if (context.adc_queue == nullptr ||
          xQueueSend(context.adc_queue, &sample, 0U) != pdPASS) {
        set_error(context, k_error_adc_queue);
      }
    } else {
      set_error(context, k_error_adc_read);
    }

    vTaskDelayUntil(&last_wake_time, k_adc_period);
  }
}
}  // namespace

void start_adc_smoke(SmokeContext& context) noexcept {
  mark_result(context, ru::driver::Adc::start(), k_error_adc_init);
  mark_result(context, context.pot.init(), k_error_adc_init);

  context.adc_queue =
      xQueueCreateStatic(k_adc_queue_length, sizeof(AdcSample), g_adc_queue_storage,
                         &g_adc_queue_control);
  if (context.adc_queue == nullptr) {
    set_error(context, k_error_adc_queue);
    return;
  }

  if (g_adc_task_handle != nullptr) {
    return;
  }

  g_adc_task_handle =
      xTaskCreateStatic(adc_task, "smoke_adc", k_adc_task_stack_depth, &context,
                        k_adc_task_priority, g_adc_task_stack, &g_adc_task_tcb);
  if (g_adc_task_handle == nullptr) {
    set_error(context, k_error_task_create);
  }
}

}  // namespace smoke
