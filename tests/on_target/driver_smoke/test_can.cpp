#include "smoke_test.hpp"

#include <cstring>

namespace smoke {
namespace {
uint32_t g_heartbeat_counter{0U};

void put_u16(uint8_t* const data, const uint16_t value) noexcept {
  data[0] = static_cast<uint8_t>(value);
  data[1] = static_cast<uint8_t>(value >> 8U);
}

void put_u32(uint8_t* const data, const uint32_t value) noexcept {
  data[0] = static_cast<uint8_t>(value);
  data[1] = static_cast<uint8_t>(value >> 8U);
  data[2] = static_cast<uint8_t>(value >> 16U);
  data[3] = static_cast<uint8_t>(value >> 24U);
}

void send_can(SmokeContext& context, ru::driver::M_can& can,
              const uint32_t id, const uint8_t* const payload,
              const uint8_t len, const uint32_t error_bit) noexcept {
  if (!context.can_ready) {
    return;
  }

  const auto message = ru::driver::can_message::classic_standard(id, payload, len);
  if (!message.has_value()) {
    set_error(context, error_bit);
    return;
  }

  mark_result(context, can.try_write(message.value()), error_bit);
}

void echo_rx(SmokeContext& context, ru::driver::M_can& can,
             const ru::driver::M_fifo fifo, const uint8_t source,
             const uint32_t reply_id, const uint32_t error_bit) noexcept {
  const auto received = can.try_read(fifo);
  if (!received.has_value()) {
    return;
  }

  const auto frame = ru::driver::make_can_frame_view(received.value().message);
  uint8_t payload[8]{};
  payload[0] = source;
  put_u16(&payload[1], static_cast<uint16_t>(frame.id.value & 0x7FFU));
  payload[3] = frame.len;
  const auto copy_len = frame.len < 4U ? frame.len : 4U;
  std::memcpy(&payload[4], frame.data, copy_len);
  send_can(context, can, reply_id, payload, sizeof(payload), error_bit);
}
}  // namespace

void start_can_smoke(SmokeContext& context) noexcept {
  mark_result(context, ru::driver::M_can::start(), k_error_can_config);

  ru::driver::CanControllerConfig config{};
  config.max_frame_format = ru::driver::CanFrameFormat::Classic;
  config.standard_filter_count = 4U;
  config.extended_filter_count = 0U;

  mark_result(context,
              ru::driver::M_can::configure(ru::driver::M_canId::CAN_1, config),
              k_error_can_config);
  mark_result(context,
              ru::driver::M_can::configure(ru::driver::M_canId::CAN_2, config),
              k_error_can_config);
  mark_result(context, context.can1.init(), k_error_can1_init);
  mark_result(context, context.can2.init(), k_error_can2_init);
  mark_result(context, context.can1.set_not_matching(ru::driver::M_fifo::FIFO0),
              k_error_can1_init);
  mark_result(context, context.can2.set_not_matching(ru::driver::M_fifo::FIFO0),
              k_error_can2_init);

  context.can_ready =
      (error_bits(context) & (k_error_can_config | k_error_can1_init |
                             k_error_can2_init)) == 0U;
}

void can_report_adc(SmokeContext& context, const AdcSample& sample) noexcept {
  uint8_t payload[8]{};
  put_u16(&payload[0], sample.value);
  put_u16(&payload[2], static_cast<uint16_t>(sample.counter));
  put_u16(&payload[4], static_cast<uint16_t>(error_bits(context)));
  payload[6] = static_cast<uint8_t>(context.eeprom_summary.readback_ok);
  payload[7] = 0U;

  send_can(context, context.can1, k_can_id_adc, payload, sizeof(payload),
           k_error_can1_tx);
  send_can(context, context.can2, k_can_id_adc, payload, sizeof(payload),
           k_error_can2_tx);
}

void can_report_eeprom(SmokeContext& context) noexcept {
  uint8_t payload[8]{};
  payload[0] = context.eeprom_summary.valid_previous;
  payload[1] = context.eeprom_summary.readback_ok;
  put_u16(&payload[2], static_cast<uint16_t>(context.eeprom_summary.boot_counter));
  put_u16(&payload[4],
          static_cast<uint16_t>(context.eeprom_summary.previous_boot_counter));
  put_u16(&payload[6], static_cast<uint16_t>(context.eeprom_summary.checksum));

  send_can(context, context.can1, k_can_id_eeprom, payload, sizeof(payload),
           k_error_can1_tx);
  send_can(context, context.can2, k_can_id_eeprom, payload, sizeof(payload),
           k_error_can2_tx);
}

void can_report_heartbeat(SmokeContext& context) noexcept {
  uint8_t payload[8]{};
  put_u16(&payload[0], static_cast<uint16_t>(g_heartbeat_counter++));
  put_u16(&payload[2], static_cast<uint16_t>(error_bits(context)));
  put_u16(&payload[4], static_cast<uint16_t>(context.freertos_queue_counter));
  put_u16(&payload[6], static_cast<uint16_t>(context.freertos_timer_counter));

  send_can(context, context.can1, k_can_id_heartbeat, payload, sizeof(payload),
           k_error_can1_tx);
  send_can(context, context.can2, k_can_id_heartbeat, payload, sizeof(payload),
           k_error_can2_tx);
}

void can_service_rx(SmokeContext& context) noexcept {
  echo_rx(context, context.can1, ru::driver::M_fifo::FIFO0, 1U,
          k_can_id_can1_echo, k_error_can1_tx);
  echo_rx(context, context.can2, ru::driver::M_fifo::FIFO0, 2U,
          k_can_id_can2_echo, k_error_can2_tx);
}

}  // namespace smoke
