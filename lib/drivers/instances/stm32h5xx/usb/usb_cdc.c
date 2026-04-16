#include "usb_cdc.h"

#include <stdio.h>
#include <string.h>

#include "freertos_stm32_config.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"
#include "ux_device_stack.h"
#include "ux_dcd_stm32.h"
#include "ux_system.h"

#define RU_USB_CDC_PACKET_SIZE 64U
#define RU_USB_CDC_RX_BUFFER_SIZE 1024U
#define RU_USB_CDC_TX_BUFFER_SIZE 1024U
#define RU_USB_CDC_TASK_STACK_WORDS 512U
#define RU_USB_CDC_TASK_PRIORITY (tskIDLE_PRIORITY + 3U)
#define RU_USBX_MEMORY_SIZE (12U * 1024U)

#define RU_USB_VID 0x0483U
#define RU_USB_PID 0x5710U
#define RU_USB_BCD_DEVICE 0x0100U

#ifndef RU_USB_MANUFACTURER_STRING
#define RU_USB_MANUFACTURER_STRING "Template"
#endif

#ifndef RU_USB_PRODUCT_STRING
#define RU_USB_PRODUCT_STRING "FreeRTOS USB CDC"
#endif

typedef enum {
  RU_USB_CDC_IO_STATE_RESET = 0,
  RU_USB_CDC_IO_STATE_READ,
  RU_USB_CDC_IO_STATE_WRITE,
} ru_usb_cdc_io_state_t;

typedef struct {
  UX_SLAVE_CLASS_CDC_ACM* instance;
  UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER line_coding;
  UX_SLAVE_CLASS_CDC_ACM_LINE_STATE_PARAMETER line_state;
  StreamBufferHandle_t rx_stream;
  StreamBufferHandle_t tx_stream;
  TaskHandle_t task_handle;
  ULONG rx_actual_length;
  ULONG tx_actual_length;
  size_t tx_packet_length;
  ru_usb_cdc_io_state_t rx_state;
  ru_usb_cdc_io_state_t tx_state;
  uint8_t pcd_started;
  uint8_t enabled;
  uint8_t started;
  uint8_t initialized;
} ru_usb_cdc_context_t;

static ru_usb_cdc_context_t g_usb_cdc = {
    .line_coding =
        {
            .ux_slave_class_cdc_acm_parameter_baudrate = 115200UL,
            .ux_slave_class_cdc_acm_parameter_stop_bit = 0U,
            .ux_slave_class_cdc_acm_parameter_parity = 0U,
            .ux_slave_class_cdc_acm_parameter_data_bit = 8U,
        },
};

__ALIGN_BEGIN static uint8_t g_usbx_memory[RU_USBX_MEMORY_SIZE] __ALIGN_END;
__ALIGN_BEGIN static uint8_t g_rx_packet[RU_USB_CDC_PACKET_SIZE] __ALIGN_END;
__ALIGN_BEGIN static uint8_t g_tx_packet[RU_USB_CDC_PACKET_SIZE] __ALIGN_END;
__ALIGN_BEGIN static uint8_t g_rx_stream_storage[RU_USB_CDC_RX_BUFFER_SIZE] __ALIGN_END;
__ALIGN_BEGIN static uint8_t g_tx_stream_storage[RU_USB_CDC_TX_BUFFER_SIZE] __ALIGN_END;
static StaticStreamBuffer_t g_rx_stream_tcb;
static StaticStreamBuffer_t g_tx_stream_tcb;
static StackType_t g_usb_cdc_task_stack[RU_USB_CDC_TASK_STACK_WORDS];
static StaticTask_t g_usb_cdc_task_tcb;

static UCHAR g_language_id_framework[] = {0x09U, 0x04U};
static UCHAR g_string_framework[96];
static ULONG g_string_framework_length;

static UCHAR g_device_framework_full_speed[] = {
    0x12, 0x01, 0x00, 0x02, 0xEF, 0x02, 0x01, 0x40,
    (UCHAR)(RU_USB_VID & 0xFFU), (UCHAR)(RU_USB_VID >> 8),
    (UCHAR)(RU_USB_PID & 0xFFU), (UCHAR)(RU_USB_PID >> 8),
    (UCHAR)(RU_USB_BCD_DEVICE & 0xFFU), (UCHAR)(RU_USB_BCD_DEVICE >> 8),
    0x01, 0x02, 0x03, 0x01,

    0x09, 0x02, 0x4BU, 0x00, 0x02, 0x01, 0x00, 0x80, 0x32,

    0x08, 0x0BU, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00,

    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    0x05, 0x24, 0x00, 0x10, 0x01,
    0x04, 0x24, 0x02, 0x02,
    0x05, 0x24, 0x06, 0x00, 0x01,
    0x05, 0x24, 0x01, 0x00, 0x01,
    0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0x10,

    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
};

static UCHAR g_device_framework_high_speed[] = {
    0x12, 0x01, 0x00, 0x02, 0xEF, 0x02, 0x01, 0x40,
    (UCHAR)(RU_USB_VID & 0xFFU), (UCHAR)(RU_USB_VID >> 8),
    (UCHAR)(RU_USB_PID & 0xFFU), (UCHAR)(RU_USB_PID >> 8),
    (UCHAR)(RU_USB_BCD_DEVICE & 0xFFU), (UCHAR)(RU_USB_BCD_DEVICE >> 8),
    0x01, 0x02, 0x03, 0x01,

    0x0A, 0x06, 0x00, 0x02, 0xEF, 0x02, 0x01, 0x40, 0x01, 0x00,

    0x09, 0x02, 0x4BU, 0x00, 0x02, 0x01, 0x00, 0x80, 0x32,

    0x08, 0x0BU, 0x00, 0x02, 0x02, 0x02, 0x01, 0x00,

    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    0x05, 0x24, 0x00, 0x10, 0x01,
    0x04, 0x24, 0x02, 0x02,
    0x05, 0x24, 0x06, 0x00, 0x01,
    0x05, 0x24, 0x01, 0x00, 0x01,
    0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0x10,

    0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
};

PCD_HandleTypeDef hpcd_USB_DRD_FS;

static void ru_usb_append_string(UCHAR index, const char* text, size_t* offset) {
  const size_t length = strlen(text);

  if ((*offset + 4U + length) > sizeof(g_string_framework)) {
    return;
  }

  g_string_framework[(*offset)++] = 0x09U;
  g_string_framework[(*offset)++] = 0x04U;
  g_string_framework[(*offset)++] = index;
  g_string_framework[(*offset)++] = (UCHAR)length;
  memcpy(&g_string_framework[*offset], text, length);
  *offset += length;
}

static void ru_usb_build_string_framework(void) {
  char serial_number[25];
  size_t offset = 0U;
  const int serial_length = snprintf(serial_number, sizeof(serial_number), "%08lX%08lX%08lX",
                                     (unsigned long)HAL_GetUIDw0(),
                                     (unsigned long)HAL_GetUIDw1(),
                                     (unsigned long)HAL_GetUIDw2());

  memset(g_string_framework, 0, sizeof(g_string_framework));
  ru_usb_append_string(0x01U, RU_USB_MANUFACTURER_STRING, &offset);
  ru_usb_append_string(0x02U, RU_USB_PRODUCT_STRING, &offset);

  if (serial_length > 0) {
    ru_usb_append_string(0x03U, serial_number, &offset);
  } else {
    ru_usb_append_string(0x03U, "00000000", &offset);
  }

  g_string_framework_length = (ULONG)offset;
}

static void ru_usb_cdc_reset_io_state(void) {
  g_usb_cdc.rx_state = RU_USB_CDC_IO_STATE_RESET;
  g_usb_cdc.tx_state = RU_USB_CDC_IO_STATE_RESET;
  g_usb_cdc.rx_actual_length = 0U;
  g_usb_cdc.tx_actual_length = 0U;
  g_usb_cdc.tx_packet_length = 0U;
}

static VOID ru_usb_cdc_activate(VOID* cdc_instance) {
  g_usb_cdc.instance = (UX_SLAVE_CLASS_CDC_ACM*)cdc_instance;
#ifndef UX_DEVICE_CLASS_CDC_ACM_TRANSMISSION_DISABLE
  g_usb_cdc.instance->ux_slave_class_cdc_acm_transmission_status = UX_FALSE;
  g_usb_cdc.instance->ux_slave_class_cdc_acm_scheduled_write = UX_FALSE;
  g_usb_cdc.instance->ux_device_class_cdc_acm_read_state = UX_STATE_RESET;
  g_usb_cdc.instance->ux_device_class_cdc_acm_write_state = UX_STATE_RESET;
#endif
  (void)ux_device_class_cdc_acm_ioctl(
      g_usb_cdc.instance, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING, &g_usb_cdc.line_coding);
  ru_usb_cdc_reset_io_state();
}

static VOID ru_usb_cdc_deactivate(VOID* cdc_instance) {
  UX_PARAMETER_NOT_USED(cdc_instance);
  g_usb_cdc.instance = UX_NULL;
  ru_usb_cdc_reset_io_state();
}

static VOID ru_usb_cdc_parameter_change(VOID* cdc_instance) {
  UX_PARAMETER_NOT_USED(cdc_instance);

  if (g_usb_cdc.instance == UX_NULL) {
    return;
  }

  (void)ux_device_class_cdc_acm_ioctl(
      g_usb_cdc.instance, UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_CODING, &g_usb_cdc.line_coding);
  (void)ux_device_class_cdc_acm_ioctl(
      g_usb_cdc.instance, UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_STATE, &g_usb_cdc.line_state);
}

static int ru_usb_cdc_is_configured(void) {
  return (g_usb_cdc.instance != UX_NULL &&
          g_usb_cdc.instance->ux_slave_class_cdc_acm_interface != UX_NULL &&
          _ux_system_slave->ux_system_slave_device.ux_slave_device_state ==
              UX_DEVICE_CONFIGURED)
             ? 1
             : 0;
}

static void ru_usb_cdc_tasks_run(void) {
  UX_SLAVE_CLASS* class_instance;
  UX_SLAVE_DCD* dcd;
  ULONG class_index;

  dcd = &_ux_system_slave->ux_system_slave_dcd;
  (void)dcd->ux_slave_dcd_function(dcd, UX_DCD_TASKS_RUN, UX_NULL);

  if (g_usb_cdc.instance == UX_NULL ||
      g_usb_cdc.instance->ux_slave_class_cdc_acm_interface == UX_NULL) {
    return;
  }

  class_instance = _ux_system_slave->ux_system_slave_class_array;
  for (class_index = 0U; class_index < UX_SYSTEM_DEVICE_MAX_CLASS_GET();
       ++class_index) {
    if (class_instance->ux_slave_class_status != UX_UNUSED &&
        class_instance->ux_slave_class_task_function != UX_NULL) {
      (void)class_instance->ux_slave_class_task_function(
          class_instance->ux_slave_class_instance);
    }

    ++class_instance;
  }
}

static int ru_usb_cdc_stack_init(void) {
  UX_SLAVE_CLASS_CDC_ACM_PARAMETER parameter;

  memset(&parameter, 0, sizeof(parameter));
  memset(&hpcd_USB_DRD_FS, 0, sizeof(hpcd_USB_DRD_FS));

  ru_usb_build_string_framework();

  if (ux_system_initialize(g_usbx_memory, sizeof(g_usbx_memory), UX_NULL, 0U) != UX_SUCCESS) {
    return -1;
  }

  if (ux_device_stack_initialize(g_device_framework_high_speed, sizeof(g_device_framework_high_speed),
                                 g_device_framework_full_speed, sizeof(g_device_framework_full_speed),
                                 g_string_framework, g_string_framework_length,
                                 g_language_id_framework, sizeof(g_language_id_framework),
                                 UX_NULL) != UX_SUCCESS) {
    return -1;
  }

  parameter.ux_slave_class_cdc_acm_instance_activate = ru_usb_cdc_activate;
  parameter.ux_slave_class_cdc_acm_instance_deactivate = ru_usb_cdc_deactivate;
  parameter.ux_slave_class_cdc_acm_parameter_change = ru_usb_cdc_parameter_change;

  if (ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry,
                                     1U, 0U, &parameter) != UX_SUCCESS) {
    return -1;
  }

  hpcd_USB_DRD_FS.Instance = USB_DRD_FS;
  hpcd_USB_DRD_FS.Init.dev_endpoints = 8U;
  hpcd_USB_DRD_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_DRD_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_DRD_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.battery_charging_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.vbus_sensing_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.bulk_doublebuffer_enable = DISABLE;
  hpcd_USB_DRD_FS.Init.iso_singlebuffer_enable = DISABLE;

  if (HAL_PCD_Init(&hpcd_USB_DRD_FS) != HAL_OK) {
    return -1;
  }

  if (HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x00U, PCD_SNG_BUF, 0x20U) != HAL_OK ||
      HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x80U, PCD_SNG_BUF, 0x60U) != HAL_OK ||
      HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x81U, PCD_SNG_BUF, 0xA0U) != HAL_OK ||
      HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x02U, PCD_SNG_BUF, 0xE0U) != HAL_OK ||
      HAL_PCDEx_PMAConfig(&hpcd_USB_DRD_FS, 0x83U, PCD_SNG_BUF, 0x120U) != HAL_OK) {
    return -1;
  }

  if (_ux_dcd_stm32_initialize((ULONG)USB_DRD_FS, (ULONG)&hpcd_USB_DRD_FS) != UX_SUCCESS) {
    return -1;
  }

  ru_usb_cdc_reset_io_state();
  g_usb_cdc.initialized = 1U;
  return 0;
}

static void ru_usb_cdc_service_rx(void) {
  UINT status;

  switch (g_usb_cdc.rx_state) {
    case RU_USB_CDC_IO_STATE_RESET:
      (void)ux_device_class_cdc_acm_ioctl(g_usb_cdc.instance, UX_SLAVE_CLASS_CDC_ACM_IOCTL_ABORT_PIPE,
                                          (VOID*)UX_SLAVE_CLASS_CDC_ACM_ENDPOINT_RCV);
      g_usb_cdc.rx_state = RU_USB_CDC_IO_STATE_READ;
      break;

    case RU_USB_CDC_IO_STATE_READ:
      status = ux_device_class_cdc_acm_read_run(g_usb_cdc.instance, g_rx_packet, sizeof(g_rx_packet),
                                                &g_usb_cdc.rx_actual_length);
      if (status == UX_STATE_NEXT) {
        if (g_usb_cdc.rx_actual_length != 0U) {
          (void)xStreamBufferSend(g_usb_cdc.rx_stream, g_rx_packet,
                                  (size_t)g_usb_cdc.rx_actual_length, 0U);
        }
      } else if (status < UX_STATE_NEXT) {
        g_usb_cdc.rx_state = RU_USB_CDC_IO_STATE_RESET;
      }
      break;

    default:
      g_usb_cdc.rx_state = RU_USB_CDC_IO_STATE_RESET;
      break;
  }
}

static void ru_usb_cdc_service_tx(void) {
  UINT status;

  switch (g_usb_cdc.tx_state) {
    case RU_USB_CDC_IO_STATE_RESET:
      (void)ux_device_class_cdc_acm_ioctl(g_usb_cdc.instance, UX_SLAVE_CLASS_CDC_ACM_IOCTL_ABORT_PIPE,
                                          (VOID*)UX_SLAVE_CLASS_CDC_ACM_ENDPOINT_XMIT);
      g_usb_cdc.tx_state = RU_USB_CDC_IO_STATE_WRITE;
      break;

    case RU_USB_CDC_IO_STATE_WRITE:
      if (g_usb_cdc.tx_packet_length == 0U) {
        g_usb_cdc.tx_packet_length =
            xStreamBufferReceive(g_usb_cdc.tx_stream, g_tx_packet, sizeof(g_tx_packet), 0U);
        if (g_usb_cdc.tx_packet_length == 0U) {
          break;
        }
      }

      status = ux_device_class_cdc_acm_write_run(g_usb_cdc.instance, g_tx_packet, g_usb_cdc.tx_packet_length,
                                                 &g_usb_cdc.tx_actual_length);
      if (status == UX_STATE_NEXT) {
        g_usb_cdc.tx_packet_length = 0U;
      } else if (status < UX_STATE_NEXT) {
        g_usb_cdc.tx_packet_length = 0U;
        g_usb_cdc.tx_state = RU_USB_CDC_IO_STATE_RESET;
      }
      break;

    default:
      g_usb_cdc.tx_state = RU_USB_CDC_IO_STATE_RESET;
      break;
  }
}

static void ru_usb_cdc_task(void* context) {
  (void)context;

  for (;;) {
    if (g_usb_cdc.enabled == 0U) {
      if (g_usb_cdc.pcd_started != 0U) {
        (void)HAL_PCD_Stop(&hpcd_USB_DRD_FS);
        g_usb_cdc.pcd_started = 0U;
      }

      ru_usb_cdc_reset_io_state();
      vTaskDelay(pdMS_TO_TICKS(10U));
      continue;
    }

    if (g_usb_cdc.pcd_started == 0U) {
      if (HAL_PCD_Start(&hpcd_USB_DRD_FS) == HAL_OK) {
        g_usb_cdc.pcd_started = 1U;
      } else {
        vTaskDelay(pdMS_TO_TICKS(10U));
        continue;
      }
    }

    ru_usb_cdc_tasks_run();

    if (ru_usb_cdc_is_configured() != 0) {
      ru_usb_cdc_service_rx();
      ru_usb_cdc_service_tx();
    }

    vTaskDelay(pdMS_TO_TICKS(1U));
  }
}

static TickType_t ru_usb_timeout_ticks(uint32_t timeout_ms) {
  if (timeout_ms == 0U) {
    return 0U;
  }

  return pdMS_TO_TICKS(timeout_ms);
}

static int ru_usb_stream_write(StreamBufferHandle_t stream, const uint8_t* data, size_t len,
                               TickType_t timeout_ticks) {
  size_t written = 0U;
  TickType_t start_tick = xTaskGetTickCount();

  while (written < len) {
    const TickType_t now = xTaskGetTickCount();
    const TickType_t elapsed = now - start_tick;
    const TickType_t remaining = (elapsed >= timeout_ticks) ? 0U : (timeout_ticks - elapsed);
    const size_t chunk = xStreamBufferSend(stream, &data[written], len - written, remaining);

    if (chunk == 0U) {
      return -1;
    }

    written += chunk;
  }

  return 0;
}

static int ru_usb_stream_read(StreamBufferHandle_t stream, uint8_t* data, size_t len,
                              TickType_t timeout_ticks) {
  size_t read = 0U;
  TickType_t start_tick = xTaskGetTickCount();

  while (read < len) {
    const TickType_t now = xTaskGetTickCount();
    const TickType_t elapsed = now - start_tick;
    const TickType_t remaining = (elapsed >= timeout_ticks) ? 0U : (timeout_ticks - elapsed);
    const size_t chunk = xStreamBufferReceive(stream, &data[read], len - read, remaining);

    if (chunk == 0U) {
      return -1;
    }

    read += chunk;
  }

  return 0;
}

int ru_stm32_usb_cdc_start(void) {
  if (g_usb_cdc.started != 0U) {
    g_usb_cdc.enabled = 1U;
    return 0;
  }

  g_usb_cdc.rx_stream = xStreamBufferCreateStatic(RU_USB_CDC_RX_BUFFER_SIZE, 1U,
                                                  g_rx_stream_storage,
                                                  &g_rx_stream_tcb);
  g_usb_cdc.tx_stream = xStreamBufferCreateStatic(RU_USB_CDC_TX_BUFFER_SIZE, 1U,
                                                  g_tx_stream_storage,
                                                  &g_tx_stream_tcb);
  if ((g_usb_cdc.rx_stream == NULL) || (g_usb_cdc.tx_stream == NULL)) {
    return -1;
  }

  if (ru_usb_cdc_stack_init() != 0) {
    return -1;
  }

  g_usb_cdc.enabled = 1U;
  g_usb_cdc.task_handle = xTaskCreateStatic(ru_usb_cdc_task, "usb-cdc",
                                            RU_USB_CDC_TASK_STACK_WORDS, NULL,
                                            RU_USB_CDC_TASK_PRIORITY,
                                            g_usb_cdc_task_stack,
                                            &g_usb_cdc_task_tcb);
  if (g_usb_cdc.task_handle == NULL) {
    g_usb_cdc.enabled = 0U;
    return -1;
  }

  g_usb_cdc.started = 1U;
  return 0;
}

int ru_stm32_usb_cdc_init(void) {
  return (g_usb_cdc.initialized != 0U) ? 0 : -1;
}

int ru_stm32_usb_cdc_stop(void) {
  if (g_usb_cdc.initialized == 0U) {
    return 0;
  }

  g_usb_cdc.enabled = 0U;
  g_usb_cdc.instance = UX_NULL;
  ru_usb_cdc_reset_io_state();
  if (g_usb_cdc.pcd_started != 0U) {
    (void)HAL_PCD_Stop(&hpcd_USB_DRD_FS);
    g_usb_cdc.pcd_started = 0U;
  }
  return 0;
}

int ru_stm32_usb_cdc_write(const uint8_t* data, size_t len, uint32_t timeout_ms) {
  if ((len != 0U) && (data == NULL)) {
    return -1;
  }

  if (len == 0U) {
    return 0;
  }

  if ((g_usb_cdc.enabled == 0U) || (g_usb_cdc.tx_stream == NULL)) {
    return -1;
  }

  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    return -1;
  }

  return ru_usb_stream_write(g_usb_cdc.tx_stream, data, len, ru_usb_timeout_ticks(timeout_ms));
}

int ru_stm32_usb_cdc_read(uint8_t* data, size_t len, uint32_t timeout_ms) {
  if ((len != 0U) && (data == NULL)) {
    return -1;
  }

  if (len == 0U) {
    return 0;
  }

  if ((g_usb_cdc.enabled == 0U) || (g_usb_cdc.rx_stream == NULL)) {
    return -1;
  }

  if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
    return -1;
  }

  return ru_usb_stream_read(g_usb_cdc.rx_stream, data, len, ru_usb_timeout_ticks(timeout_ms));
}
