#include "smoke_test.hpp"

#include "task.h"
#include "timers.h"

namespace smoke {
namespace {
constexpr uint32_t k_led_task_stack_depth = 256U;
constexpr uint32_t k_queue_task_stack_depth = 384U;
constexpr UBaseType_t k_led_task_priority = tskIDLE_PRIORITY + 1U;
constexpr UBaseType_t k_queue_task_priority = tskIDLE_PRIORITY + 1U;
constexpr UBaseType_t k_queue_length = 4U;

StaticTask_t g_led_task_tcb;
StaticTask_t g_queue_tx_task_tcb;
StaticTask_t g_queue_rx_task_tcb;
StackType_t g_led_task_stack[k_led_task_stack_depth];
StackType_t g_queue_tx_task_stack[k_queue_task_stack_depth];
StackType_t g_queue_rx_task_stack[k_queue_task_stack_depth];
TaskHandle_t g_led_task_handle = nullptr;
TaskHandle_t g_queue_tx_task_handle = nullptr;
TaskHandle_t g_queue_rx_task_handle = nullptr;

StaticQueue_t g_queue_control;
uint8_t g_queue_storage[k_queue_length * sizeof(uint32_t)];
QueueHandle_t g_queue = nullptr;

StaticTimer_t g_timer_control;
TimerHandle_t g_timer = nullptr;

void led_task(void* arg) {
  auto& context = *static_cast<SmokeContext*>(arg);

  for (;;) {
    (void)context.led.toggle();
    const TickType_t period =
        error_bits(context) == 0U ? pdMS_TO_TICKS(500U) : pdMS_TO_TICKS(100U);
    vTaskDelay(period);
  }
}

void queue_tx_task(void* arg) {
  auto& context = *static_cast<SmokeContext*>(arg);
  uint32_t value = 0U;

  for (;;) {
    if (g_queue == nullptr || xQueueSend(g_queue, &value, 0U) != pdPASS) {
      set_error(context, k_error_freertos_queue);
    }
    ++value;
    vTaskDelay(pdMS_TO_TICKS(50U));
  }
}

void queue_rx_task(void* arg) {
  auto& context = *static_cast<SmokeContext*>(arg);
  uint32_t last_value = 0U;
  bool first = true;

  for (;;) {
    uint32_t value = 0U;
    if (g_queue == nullptr ||
        xQueueReceive(g_queue, &value, pdMS_TO_TICKS(200U)) != pdPASS) {
      set_error(context, k_error_freertos_queue);
      continue;
    }

    if (!first && value <= last_value) {
      set_error(context, k_error_freertos_queue);
    }

    first = false;
    last_value = value;
    context.freertos_queue_counter = value;
  }
}

void timer_callback(TimerHandle_t timer) {
  auto* context = static_cast<SmokeContext*>(pvTimerGetTimerID(timer));
  if (context == nullptr) {
    return;
  }

  ++context->freertos_timer_counter;
}
}  // namespace

void start_freertos_smoke(SmokeContext& context) noexcept {
  mark_result(context, ru::driver::Gpio::start(), k_error_led_init);
  mark_result(context, context.led.init(), k_error_led_init);

  g_queue = xQueueCreateStatic(k_queue_length, sizeof(uint32_t), g_queue_storage,
                               &g_queue_control);
  if (g_queue == nullptr) {
    set_error(context, k_error_freertos_queue);
  }

  g_timer = xTimerCreateStatic("smoke_timer", pdMS_TO_TICKS(250U), pdTRUE,
                               &context, timer_callback, &g_timer_control);
  if (g_timer == nullptr || xTimerStart(g_timer, 0U) != pdPASS) {
    set_error(context, k_error_freertos_timer);
  }

  if (g_led_task_handle == nullptr) {
    g_led_task_handle =
        xTaskCreateStatic(led_task, "smoke_led", k_led_task_stack_depth, &context,
                          k_led_task_priority, g_led_task_stack, &g_led_task_tcb);
  }
  if (g_queue_tx_task_handle == nullptr) {
    g_queue_tx_task_handle =
        xTaskCreateStatic(queue_tx_task, "smoke_q_tx", k_queue_task_stack_depth,
                          &context, k_queue_task_priority, g_queue_tx_task_stack,
                          &g_queue_tx_task_tcb);
  }
  if (g_queue_rx_task_handle == nullptr) {
    g_queue_rx_task_handle =
        xTaskCreateStatic(queue_rx_task, "smoke_q_rx", k_queue_task_stack_depth,
                          &context, k_queue_task_priority, g_queue_rx_task_stack,
                          &g_queue_rx_task_tcb);
  }

  if (g_led_task_handle == nullptr || g_queue_tx_task_handle == nullptr ||
      g_queue_rx_task_handle == nullptr) {
    set_error(context, k_error_task_create);
  }
}

}  // namespace smoke
