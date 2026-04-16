#include "can/flex_can.hpp"

#include "can_helpers.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Flex_can::Flex_can(const Flex_canId id) noexcept : m_id(id) {
  LOG("Flex_can " << toText(m_id) << " create");
}

result Flex_can::start() noexcept {
  LOG("Flex_can driver start");
  return result::OK;
}

result Flex_can::init() noexcept {
  LOG("Flex_can " << toText(m_id) << " init");
  return result::OK;
}

result Flex_can::stop() noexcept {
  LOG("Flex_can " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<CanMessageTs, result> Flex_can::read() noexcept {
  LOG("Flex_can " << toText(m_id) << " read");
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Flex_can::read(uint8_t mb) noexcept {
  LOG("Flex_can " << toText(m_id) << " read mb=" << raw_value(mb));
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Flex_can::try_read() noexcept {
  LOG("Flex_can " << toText(m_id) << " try_read");
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> Flex_can::try_read(uint8_t mb) noexcept {
  LOG("Flex_can " << toText(m_id) << " try_read mb=" << raw_value(mb));
  return make_dummy_can_message_ts();
}

result Flex_can::write(const CanMessage& message) noexcept {
  (void)message;
  LOG("Flex_can " << toText(m_id) << " write");
  return result::OK;
}

result Flex_can::try_write(const CanMessage& message) noexcept {
  (void)message;
  LOG("Flex_can " << toText(m_id) << " try_write");
  return result::OK;
}

result Flex_can::set_rx_callback(void (*callback)(CanMessageTs)) {
  (void)callback;
  LOG("Flex_can " << toText(m_id) << " set_rx_callback");
  return result::OK;
}

result Flex_can::enable_rx_interrupt(uint8_t id) {
  LOG("Flex_can " << toText(m_id) << " enable_rx_interrupt id=" << raw_value(id));
  return result::OK;
}

result Flex_can::disable_rx_interrupt(uint8_t id) {
  LOG("Flex_can " << toText(m_id) << " disable_rx_interrupt id=" << raw_value(id));
  return result::OK;
}

expected::expected<Timestamp, result> Flex_can::get_timestamp() {
  LOG("Flex_can " << toText(m_id) << " get_timestamp");
  return static_cast<Timestamp>(0U);
}

result Flex_can::set_rx_priority(uint8_t priority) {
  LOG("Flex_can " << toText(m_id) << " set_rx_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Flex_can::get_rx_priority() {
  LOG("Flex_can " << toText(m_id) << " get_rx_priority");
  return static_cast<uint8_t>(0U);
}

result Flex_can::set_rx_interrupt(bool on) {
  LOG("Flex_can " << toText(m_id) << " set_rx_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Flex_can::is_rx_interrupt_on() {
  LOG("Flex_can " << toText(m_id) << " is_rx_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result Flex_can::set_error_priority(uint8_t priority) {
  LOG("Flex_can " << toText(m_id) << " set_error_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Flex_can::get_error_priority() {
  LOG("Flex_can " << toText(m_id) << " get_error_priority");
  return static_cast<uint8_t>(0U);
}

result Flex_can::set_error_interrupt(bool on) {
  LOG("Flex_can " << toText(m_id) << " set_error_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Flex_can::is_error_interrupt_on() {
  LOG("Flex_can " << toText(m_id) << " is_error_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result Flex_can::set_filter(Flex_filter filter, uint8_t id) {
  (void)filter;
  LOG("Flex_can " << toText(m_id) << " set_filter id=" << raw_value(id));
  return result::OK;
}

result Flex_can::enable_filter(uint8_t id) {
  LOG("Flex_can " << toText(m_id) << " enable_filter id=" << raw_value(id));
  return result::OK;
}

result Flex_can::disable_filter(uint8_t id) {
  LOG("Flex_can " << toText(m_id) << " disable_filter id=" << raw_value(id));
  return result::OK;
}

expected::expected<bool, result> Flex_can::is_filter_enabled(uint8_t id) {
  LOG("Flex_can " << toText(m_id) << " is_filter_enabled id=" << raw_value(id));
  return false;
}

result Flex_can::set_fifo_mask(uint16_t* ids, uint8_t len) {
  (void)ids;
  LOG("Flex_can " << toText(m_id) << " set_fifo_mask len=" << raw_value(len));
  return result::OK;
}

expected::expected<CanMessageTs, result> Flex_canRx::read() noexcept {
  return const_cast<Flex_can&>(m_can).read();
}

expected::expected<CanMessageTs, result> Flex_canRx::read(uint8_t mb) noexcept {
  return const_cast<Flex_can&>(m_can).read(mb);
}

expected::expected<CanMessageTs, result> Flex_canRx::try_read() noexcept {
  return const_cast<Flex_can&>(m_can).try_read();
}

expected::expected<CanMessageTs, result> Flex_canRx::try_read(uint8_t mb) noexcept {
  return const_cast<Flex_can&>(m_can).try_read(mb);
}

result Flex_canRx::set_rx_callback(void (*callback)(CanMessageTs)) {
  return const_cast<Flex_can&>(m_can).set_rx_callback(callback);
}

result Flex_canRx::enable_rx_interrupt(uint8_t id) {
  return const_cast<Flex_can&>(m_can).enable_rx_interrupt(id);
}

result Flex_canRx::disable_rx_interrupt(uint8_t id) {
  return const_cast<Flex_can&>(m_can).disable_rx_interrupt(id);
}

expected::expected<Timestamp, result> Flex_canRx::get_timestamp() {
  return const_cast<Flex_can&>(m_can).get_timestamp();
}

result Flex_canRx::set_rx_priority(uint8_t priority) {
  return const_cast<Flex_can&>(m_can).set_rx_priority(priority);
}

expected::expected<uint8_t, result> Flex_canRx::get_rx_priority() {
  return const_cast<Flex_can&>(m_can).get_rx_priority();
}

result Flex_canRx::set_rx_interrupt(bool on) {
  return const_cast<Flex_can&>(m_can).set_rx_interrupt(on);
}

expected::expected<uint8_t, result> Flex_canRx::is_rx_interrupt_on() {
  return const_cast<Flex_can&>(m_can).is_rx_interrupt_on();
}

result Flex_canRx::set_error_priority(uint8_t priority) {
  return const_cast<Flex_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Flex_canRx::get_error_priority() {
  return const_cast<Flex_can&>(m_can).get_error_priority();
}

result Flex_canRx::set_error_interrupt(bool on) {
  return const_cast<Flex_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Flex_canRx::is_error_interrupt_on() {
  return const_cast<Flex_can&>(m_can).is_error_interrupt_on();
}

result Flex_canRx::set_filter(Flex_filter filter) {
  return const_cast<Flex_can&>(m_can).set_filter(filter, filter.MB);
}

result Flex_canRx::enable_filter(uint8_t id) {
  return const_cast<Flex_can&>(m_can).enable_filter(id);
}

result Flex_canRx::disable_filter(uint8_t id) {
  return const_cast<Flex_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> Flex_canRx::is_filter_enabled(uint8_t id) {
  return const_cast<Flex_can&>(m_can).is_filter_enabled(id);
}

result Flex_canTx::write(const CanMessage& message) noexcept {
  return const_cast<Flex_can&>(m_can).write(message);
}

result Flex_canTx::try_write(const CanMessage& message) noexcept {
  return const_cast<Flex_can&>(m_can).try_write(message);
}

expected::expected<Timestamp, result> Flex_canTx::get_timestamp() {
  return const_cast<Flex_can&>(m_can).get_timestamp();
}

result Flex_canTx::set_error_priority(uint8_t priority) {
  return const_cast<Flex_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Flex_canTx::get_error_priority() {
  return const_cast<Flex_can&>(m_can).get_error_priority();
}

result Flex_canTx::set_error_interrupt(bool on) {
  return const_cast<Flex_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Flex_canTx::is_error_interrupt_on() {
  return const_cast<Flex_can&>(m_can).is_error_interrupt_on();
}

result Flex_canTx::set_filter(Flex_filter filter) {
  return const_cast<Flex_can&>(m_can).set_filter(filter, filter.MB);
}

result Flex_canTx::enable_filter(uint8_t id) {
  return const_cast<Flex_can&>(m_can).enable_filter(id);
}

result Flex_canTx::disable_filter(uint8_t id) {
  return const_cast<Flex_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> Flex_canTx::is_filter_enabled(uint8_t id) {
  return const_cast<Flex_can&>(m_can).is_filter_enabled(id);
}
}  // namespace ru::driver
