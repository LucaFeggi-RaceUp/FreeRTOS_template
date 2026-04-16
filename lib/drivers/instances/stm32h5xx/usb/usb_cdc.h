#pragma once

#include <stddef.h>
#include <stdint.h>

#include "stm32h5xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

int ru_stm32_usb_cdc_start(void);
int ru_stm32_usb_cdc_init(void);
int ru_stm32_usb_cdc_stop(void);
int ru_stm32_usb_cdc_write(const uint8_t* data, size_t len, uint32_t timeout_ms);
int ru_stm32_usb_cdc_read(uint8_t* data, size_t len, uint32_t timeout_ms);

extern PCD_HandleTypeDef hpcd_USB_DRD_FS;

#ifdef __cplusplus
}
#endif
