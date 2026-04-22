#pragma once

#include <cstdint>

#include "FreeRTOS.h"
#include "queue.h"

#include "adc.hpp"
#include "can/m_can.hpp"
#include "eeprom.hpp"
#include "gpio.hpp"
#include "serial.hpp"

namespace smoke {

inline constexpr TickType_t k_adc_period = pdMS_TO_TICKS(100U);
inline constexpr Timestamp k_usb_timeout_us = 100'000U;

inline constexpr uint32_t k_can_id_heartbeat = 0x100U;
inline constexpr uint32_t k_can_id_can1_echo = 0x111U;
inline constexpr uint32_t k_can_id_can2_echo = 0x121U;
inline constexpr uint32_t k_can_id_adc = 0x130U;
inline constexpr uint32_t k_can_id_eeprom = 0x140U;

inline constexpr uint32_t k_error_usb_init = 1UL << 0U;
inline constexpr uint32_t k_error_led_init = 1UL << 1U;
inline constexpr uint32_t k_error_adc_init = 1UL << 2U;
inline constexpr uint32_t k_error_adc_read = 1UL << 3U;
inline constexpr uint32_t k_error_adc_queue = 1UL << 4U;
inline constexpr uint32_t k_error_can_config = 1UL << 5U;
inline constexpr uint32_t k_error_can1_init = 1UL << 6U;
inline constexpr uint32_t k_error_can2_init = 1UL << 7U;
inline constexpr uint32_t k_error_can1_tx = 1UL << 8U;
inline constexpr uint32_t k_error_can2_tx = 1UL << 9U;
inline constexpr uint32_t k_error_eeprom = 1UL << 10U;
inline constexpr uint32_t k_error_freertos_queue = 1UL << 11U;
inline constexpr uint32_t k_error_freertos_timer = 1UL << 12U;
inline constexpr uint32_t k_error_task_create = 1UL << 13U;

struct AdcSample {
  uint16_t value{0U};
  uint32_t counter{0U};
  uint32_t tick_ms{0U};
};

struct EepromSummary {
  uint32_t boot_counter{0U};
  uint32_t previous_boot_counter{0U};
  uint32_t checksum{0U};
  uint8_t valid_previous{0U};
  uint8_t readback_ok{0U};
};

struct SmokeContext {
  SmokeContext() noexcept
      : led{ru::driver::GpioId::LED_E3},
        pot{ru::driver::AdcId::POT_0},
        can1{ru::driver::M_canId::CAN_1},
        can2{ru::driver::M_canId::CAN_2},
        eeprom{ru::driver::EepromId::EEPROM_0} {}

  ru::driver::Serial usb{};
  ru::driver::Gpio led;
  ru::driver::Adc pot;
  ru::driver::M_can can1;
  ru::driver::M_can can2;
  ru::driver::Eeprom eeprom;
  QueueHandle_t adc_queue{nullptr};
  uint32_t errors{0U};
  uint32_t freertos_queue_counter{0U};
  uint32_t freertos_timer_counter{0U};
  EepromSummary eeprom_summary{};
  bool usb_ready{false};
  bool can_ready{false};
};

void set_error(SmokeContext& context, uint32_t bit) noexcept;
uint32_t error_bits(SmokeContext& context) noexcept;
void mark_result(SmokeContext& context, ru::driver::result status,
                 uint32_t bit) noexcept;
void usb_line(SmokeContext& context, const char* text) noexcept;

void start_freertos_smoke(SmokeContext& context) noexcept;
void start_adc_smoke(SmokeContext& context) noexcept;
void start_can_smoke(SmokeContext& context) noexcept;
void run_eeprom_smoke(SmokeContext& context) noexcept;
void start_report_smoke(SmokeContext& context) noexcept;

void can_report_adc(SmokeContext& context, const AdcSample& sample) noexcept;
void can_report_eeprom(SmokeContext& context) noexcept;
void can_report_heartbeat(SmokeContext& context) noexcept;
void can_service_rx(SmokeContext& context) noexcept;

}  // namespace smoke
