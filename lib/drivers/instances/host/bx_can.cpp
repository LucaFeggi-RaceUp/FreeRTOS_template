#include "can/bx_can.hpp"

#include "can_helpers.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
expected::expected<CanMessage, result> Bx_canRx::read(Bx_fifo fifo) noexcept {
  return const_cast<Bx_can&>(m_can).read(fifo);
}

expected::expected<CanMessage, result> Bx_canRx::try_read(Bx_fifo fifo) noexcept {
  return const_cast<Bx_can&>(m_can).try_read(fifo);
}

result Bx_canTx::write(const CanMessage& message) noexcept {
  return const_cast<Bx_can&>(m_can).write(message);
}

result Bx_canTx::try_write(const CanMessage& message) noexcept {
  return const_cast<Bx_can&>(m_can).try_write(message);
}

Bx_can::Bx_can(const Bx_canId id) noexcept : m_id(id) {
  LOG("Bx_can " << toText(m_id) << " create");
}

result Bx_can::start() noexcept {
  LOG("Bx_can driver start");
  return result::OK;
}

result Bx_can::init() noexcept {
  LOG("Bx_can " << toText(m_id) << " init");
  return result::OK;
}

result Bx_can::stop() noexcept {
  LOG("Bx_can " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<CanMessage, result> Bx_can::read(Bx_fifo fifo) noexcept {
  LOG("Bx_can " << toText(m_id) << " read fifo=" << raw_value(fifo));
  return make_dummy_can_message();
}

expected::expected<CanMessage, result> Bx_can::try_read(Bx_fifo fifo) noexcept {
  LOG("Bx_can " << toText(m_id) << " try_read fifo=" << raw_value(fifo));
  return make_dummy_can_message();
}

result Bx_can::write(const CanMessage& message) noexcept {
  (void)message;
  LOG("Bx_can " << toText(m_id) << " write");
  return result::OK;
}

result Bx_can::try_write(const CanMessage& message) noexcept {
  (void)message;
  LOG("Bx_can " << toText(m_id) << " try_write");
  return result::OK;
}

result Bx_can::set_priority(Bx_fifo fifo, uint8_t priority) {
  LOG("Bx_can " << toText(m_id) << " set_priority fifo=" << raw_value(fifo)
                << " priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::get_priority(Bx_fifo fifo) {
  LOG("Bx_can " << toText(m_id) << " get_priority fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

result Bx_can::set_interrupt(Bx_fifo fifo, bool on) {
  LOG("Bx_can " << toText(m_id) << " set_interrupt fifo=" << raw_value(fifo)
                << " on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::is_interrupt_on(Bx_fifo fifo) {
  LOG("Bx_can " << toText(m_id) << " is_interrupt_on fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

result Bx_can::set_error_priority(uint8_t priority) {
  LOG("Bx_can " << toText(m_id) << " set_error_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::get_error_priority() {
  LOG("Bx_can " << toText(m_id) << " get_error_priority");
  return static_cast<uint8_t>(0U);
}

result Bx_can::set_error_interrupt(bool on) {
  LOG("Bx_can " << toText(m_id) << " set_error_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> Bx_can::is_error_interrupt_on() {
  LOG("Bx_can " << toText(m_id) << " is_error_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result Bx_can::set_filter(Bx_filter filter, uint8_t id) {
  (void)filter;
  LOG("Bx_can " << toText(m_id) << " set_filter id=" << raw_value(id));
  return result::OK;
}

result Bx_can::enable_filter(uint8_t id) {
  LOG("Bx_can " << toText(m_id) << " enable_filter id=" << raw_value(id));
  return result::OK;
}

result Bx_can::disable_filter(uint8_t id) {
  LOG("Bx_can " << toText(m_id) << " disable_filter id=" << raw_value(id));
  return result::OK;
}

expected::expected<bool, result> Bx_can::is_filter_enabled(uint8_t id) {
  LOG("Bx_can " << toText(m_id) << " is_filter_enabled id=" << raw_value(id));
  return false;
}

result Bx_canRx::set_priority(Bx_fifo fifo, uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_priority(fifo, priority);
}

expected::expected<uint8_t, result> Bx_canRx::get_priority(Bx_fifo fifo) {
  return const_cast<Bx_can&>(m_can).get_priority(fifo);
}

result Bx_canRx::set_interrupt(Bx_fifo fifo, bool on) {
  return const_cast<Bx_can&>(m_can).set_interrupt(fifo, on);
}

expected::expected<uint8_t, result> Bx_canRx::is_interrupt_on(Bx_fifo fifo) {
  return const_cast<Bx_can&>(m_can).is_interrupt_on(fifo);
}

result Bx_canRx::set_error_priority(uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Bx_canRx::get_error_priority() {
  return const_cast<Bx_can&>(m_can).get_error_priority();
}

result Bx_canRx::set_error_interrupt(bool on) {
  return const_cast<Bx_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Bx_canRx::is_error_interrupt_on() {
  return const_cast<Bx_can&>(m_can).is_error_interrupt_on();
}

result Bx_canRx::set_filter(Bx_filter filter, uint8_t id) {
  return const_cast<Bx_can&>(m_can).set_filter(filter, id);
}

result Bx_canRx::enable_filter(uint8_t id) {
  return const_cast<Bx_can&>(m_can).enable_filter(id);
}

result Bx_canRx::disable_filter(uint8_t id) {
  return const_cast<Bx_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> Bx_canRx::is_filter_enabled(uint8_t id) {
  return const_cast<Bx_can&>(m_can).is_filter_enabled(id);
}

result Bx_canTx::set_error_priority(uint8_t priority) {
  return const_cast<Bx_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> Bx_canTx::get_error_priority() {
  return const_cast<Bx_can&>(m_can).get_error_priority();
}

result Bx_canTx::set_error_interrupt(bool on) {
  return const_cast<Bx_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> Bx_canTx::is_error_interrupt_on() {
  return const_cast<Bx_can&>(m_can).is_error_interrupt_on();
}
}  // namespace ru::driver
