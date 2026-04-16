#include "can/multi_can.hpp"

#include "can_helpers.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
expected::expected<CanMessageTs, result> CanRx::read(uint8_t buffer) noexcept {
  return const_cast<Can&>(m_can).read(buffer);
}

expected::expected<CanMessageTs, result> CanRx::read(Multi_fifo fifo) noexcept {
  return const_cast<Can&>(m_can).read(fifo);
}

expected::expected<CanMessageTs, result> CanRx::try_read(uint8_t buffer) noexcept {
  return const_cast<Can&>(m_can).try_read(buffer);
}

expected::expected<CanMessageTs, result> CanRx::try_read(Multi_fifo fifo) noexcept {
  return const_cast<Can&>(m_can).try_read(fifo);
}

result CanTx::write(const CanMessage message) noexcept {
  return const_cast<Can&>(m_can).write(message);
}

result CanTx::try_write(const CanMessage message) noexcept {
  return const_cast<Can&>(m_can).try_write(message);
}

Can::Can(const Multi_canId id) noexcept : m_id(id) {
  LOG("Multi_can " << toText(m_id) << " create");
}

result Can::start() noexcept {
  LOG("Multi_can driver start");
  return result::OK;
}

result Can::init() noexcept {
  LOG("Multi_can " << toText(m_id) << " init");
  return result::OK;
}

result Can::stop() noexcept {
  LOG("Multi_can " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<CanMessageTs, result> Can::read(uint8_t buffer) noexcept {
  LOG("Multi_can " << toText(m_id) << " read buffer=" << raw_value(buffer));
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Can::read(Multi_fifo fifo) noexcept {
  LOG("Multi_can " << toText(m_id) << " read fifo=" << raw_value(fifo));
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Can::try_read(uint8_t buffer) noexcept {
  LOG("Multi_can " << toText(m_id) << " try_read buffer=" << raw_value(buffer));
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Can::try_read(Multi_fifo fifo) noexcept {
  LOG("Multi_can " << toText(m_id) << " try_read fifo=" << raw_value(fifo));
  return make_dummy_can_message_ts();
}

result Can::write(const CanMessage message) noexcept {
  (void)message;
  LOG("Multi_can " << toText(m_id) << " write");
  return result::OK;
}

result Can::try_write(const CanMessage message) noexcept {
  (void)message;
  LOG("Multi_can " << toText(m_id) << " try_write");
  return result::OK;
}

result Can::set_rx_callback(Multi_fifo fifo, void (*callback)(CanMessageTs)) {
  (void)callback;
  LOG("Multi_can " << toText(m_id) << " set_rx_callback fifo=" << raw_value(fifo));
  return result::OK;
}

result Can::set_rx_callback(uint8_t buffer, void (*callback)(CanMessageTs)) {
  (void)callback;
  LOG("Multi_can " << toText(m_id) << " set_rx_callback buffer=" << raw_value(buffer));
  return result::OK;
}

result Can::enable_rx_interrupt(Multi_fifo fifo) {
  LOG("Multi_can " << toText(m_id) << " enable_rx_interrupt fifo=" << raw_value(fifo));
  return result::OK;
}

result Can::enable_rx_interrupt(uint8_t buffer) {
  LOG("Multi_can " << toText(m_id) << " enable_rx_interrupt buffer=" << raw_value(buffer));
  return result::OK;
}

result Can::disable_rx_interrupt(Multi_fifo fifo) {
  LOG("Multi_can " << toText(m_id) << " disable_rx_interrupt fifo=" << raw_value(fifo));
  return result::OK;
}

result Can::disable_rx_interrupt(uint8_t buffer) {
  LOG("Multi_can " << toText(m_id) << " disable_rx_interrupt buffer=" << raw_value(buffer));
  return result::OK;
}

expected::expected<Timestamp, result> Can::get_timestamp() {
  LOG("Multi_can " << toText(m_id) << " get_timestamp");
  return static_cast<Timestamp>(0U);
}

result Can::set_rx_priority(Multi_fifo fifo, uint8_t priority) {
  LOG("Multi_can " << toText(m_id) << " set_rx_priority fifo=" << raw_value(fifo)
                   << " priority=" << raw_value(priority));
  return result::OK;
}

result Can::set_rx_priority(uint8_t priority) {
  LOG("Multi_can " << toText(m_id) << " set_rx_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Can::get_rx_priority() {
  LOG("Multi_can " << toText(m_id) << " get_rx_priority");
  return static_cast<uint8_t>(0U);
}

expected::expected<uint8_t, result> Can::get_rx_priority(Multi_fifo fifo) {
  LOG("Multi_can " << toText(m_id) << " get_rx_priority fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

result Can::set_rx_interrupt(Multi_fifo fifo, bool on) {
  LOG("Multi_can " << toText(m_id) << " set_rx_interrupt fifo=" << raw_value(fifo)
                   << " on=" << raw_value(on));
  return result::OK;
}

result Can::set_rx_interrupt(bool on) {
  LOG("Multi_can " << toText(m_id) << " set_rx_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Can::is_rx_interrupt_on(Multi_fifo fifo) {
  LOG("Multi_can " << toText(m_id) << " is_rx_interrupt_on fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

expected::expected<uint8_t, result> Can::is_rx_interrupt_on() {
  LOG("Multi_can " << toText(m_id) << " is_rx_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result Can::set_error_priority(uint8_t priority) {
  LOG("Multi_can " << toText(m_id) << " set_error_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Can::get_error_priority() {
  LOG("Multi_can " << toText(m_id) << " get_error_priority");
  return static_cast<uint8_t>(0U);
}

result Can::set_error_interrupt(bool on) {
  LOG("Multi_can " << toText(m_id) << " set_error_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Can::is_error_interrupt_on() {
  LOG("Multi_can " << toText(m_id) << " is_error_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result Can::set_filter(Multi_filter filter, uint8_t id) {
  (void)filter;
  LOG("Multi_can " << toText(m_id) << " set_filter id=" << raw_value(id));
  return result::OK;
}

result Can::enable_filter(uint8_t id) {
  LOG("Multi_can " << toText(m_id) << " enable_filter id=" << raw_value(id));
  return result::OK;
}

result Can::disable_filter(uint8_t id) {
  LOG("Multi_can " << toText(m_id) << " disable_filter id=" << raw_value(id));
  return result::OK;
}

expected::expected<bool, result> Can::is_filter_enabled(uint8_t id) {
  LOG("Multi_can " << toText(m_id) << " is_filter_enabled id=" << raw_value(id));
  return false;
}

result CanRx::set_rx_callback(Multi_fifo fifo, void (*callback)(CanMessageTs)) {
  return const_cast<Can&>(m_can).set_rx_callback(fifo, callback);
}

result CanRx::set_rx_callback(uint8_t buffer, void (*callback)(CanMessageTs)) {
  return const_cast<Can&>(m_can).set_rx_callback(buffer, callback);
}

result CanRx::enable_rx_interrupt(Multi_fifo fifo) {
  return const_cast<Can&>(m_can).enable_rx_interrupt(fifo);
}

result CanRx::enable_rx_interrupt(uint8_t buffer) {
  return const_cast<Can&>(m_can).enable_rx_interrupt(buffer);
}

result CanRx::disable_rx_interrupt(Multi_fifo fifo) {
  return const_cast<Can&>(m_can).disable_rx_interrupt(fifo);
}

result CanRx::disable_rx_interrupt(uint8_t buffer) {
  return const_cast<Can&>(m_can).disable_rx_interrupt(buffer);
}

expected::expected<Timestamp, result> CanRx::get_timestamp() {
  return const_cast<Can&>(m_can).get_timestamp();
}

result CanRx::set_rx_priority(Multi_fifo fifo, uint8_t priority) {
  return const_cast<Can&>(m_can).set_rx_priority(fifo, priority);
}

result CanRx::set_rx_priority(uint8_t priority) {
  return const_cast<Can&>(m_can).set_rx_priority(priority);
}

expected::expected<uint8_t, result> CanRx::get_rx_priority() {
  return const_cast<Can&>(m_can).get_rx_priority();
}

expected::expected<uint8_t, result> CanRx::get_rx_priority(Multi_fifo fifo) {
  return const_cast<Can&>(m_can).get_rx_priority(fifo);
}

result CanRx::set_rx_interrupt(Multi_fifo fifo, bool on) {
  return const_cast<Can&>(m_can).set_rx_interrupt(fifo, on);
}

result CanRx::set_rx_interrupt(bool on) {
  return const_cast<Can&>(m_can).set_rx_interrupt(on);
}

expected::expected<uint8_t, result> CanRx::is_rx_interrupt_on(Multi_fifo fifo) {
  return const_cast<Can&>(m_can).is_rx_interrupt_on(fifo);
}

expected::expected<uint8_t, result> CanRx::is_rx_interrupt_on() {
  return const_cast<Can&>(m_can).is_rx_interrupt_on();
}

result CanRx::set_error_priority(uint8_t priority) {
  return const_cast<Can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> CanRx::get_error_priority() {
  return const_cast<Can&>(m_can).get_error_priority();
}

result CanRx::set_error_interrupt(bool on) {
  return const_cast<Can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> CanRx::is_error_interrupt_on() {
  return const_cast<Can&>(m_can).is_error_interrupt_on();
}

result CanRx::set_filter(Multi_filter filter, uint8_t id) {
  return const_cast<Can&>(m_can).set_filter(filter, id);
}

result CanRx::enable_filter(uint8_t id) {
  return const_cast<Can&>(m_can).enable_filter(id);
}

result CanRx::disable_filter(uint8_t id) {
  return const_cast<Can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> CanRx::is_filter_enabled(uint8_t id) {
  return const_cast<Can&>(m_can).is_filter_enabled(id);
}

expected::expected<Timestamp, result> CanTx::get_timestamp() {
  return const_cast<Can&>(m_can).get_timestamp();
}

result CanTx::set_error_priority(uint8_t priority) {
  return const_cast<Can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> CanTx::get_error_priority() {
  return const_cast<Can&>(m_can).get_error_priority();
}

result CanTx::set_error_interrupt(bool on) {
  return const_cast<Can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> CanTx::is_error_interrupt_on() {
  return const_cast<Can&>(m_can).is_error_interrupt_on();
}
}  // namespace ru::driver
