#include "can/m_can.hpp"

#include <array>
#include <cstddef>

#include "can_helpers.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace {
constexpr uint8_t k_host_can_filter_slots = 28U;
constexpr M_canId k_default_m_can_id = M_canId::CAN_1;
std::array<CanControllerConfig, static_cast<std::size_t>(M_canId::COUNT)>
    g_m_can_configs{};

CanControllerConfig& host_can_config(const M_canId id) noexcept {
  const auto index = static_cast<std::size_t>(id);
  return index < g_m_can_configs.size() ? g_m_can_configs[index] : g_m_can_configs[0];
}
}  // namespace

namespace ru::driver {
expected::expected<CanMessageTs, result> M_canRx::read(M_fifo fifo) noexcept {
  return const_cast<M_can&>(m_can).read(fifo);
}

expected::expected<CanMessageTs, result> M_canRx::try_read(M_fifo fifo) noexcept {
  return const_cast<M_can&>(m_can).try_read(fifo);
}

result M_canTx::write(const CanFrameView& message) noexcept {
  return const_cast<M_can&>(m_can).write(message);
}

result M_canTx::try_write(const CanFrameView& message) noexcept {
  return const_cast<M_can&>(m_can).try_write(message);
}

M_can::M_can(const M_canId id) noexcept : m_id(id) {
  LOG("M_can " << toText(m_id) << " create");
}

result M_can::start() noexcept {
  LOG("M_can driver start");
  return result::OK;
}

result M_can::configure(const CanControllerConfig& config) noexcept {
  return configure(k_default_m_can_id, config);
}

result M_can::configure(const M_canId id, const CanControllerConfig& config) noexcept {
  if (config.standard_filter_count > k_host_can_filter_slots ||
      config.extended_filter_count > k_host_can_filter_slots) {
    return result::UNRECOVERABLE_ERROR;
  }

  host_can_config(id) = config;
  LOG("M_can " << toText(id) << " configure");
  return result::OK;
}

CanControllerConfig M_can::configuration() noexcept {
  return configuration(k_default_m_can_id);
}

CanControllerConfig M_can::configuration(const M_canId id) noexcept {
  return host_can_config(id);
}

result M_can::init() noexcept {
  LOG("M_can " << toText(m_id) << " init");
  return result::OK;
}

result M_can::stop() noexcept {
  LOG("M_can " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<CanMessageTs, result> M_can::read(M_fifo fifo) noexcept {
  LOG("M_can " << toText(m_id) << " read fifo=" << raw_value(fifo));
  return make_dummy_can_message_ts();
}

expected::expected<CanMessageTs, result> M_can::try_read(M_fifo fifo) noexcept {
  LOG("M_can " << toText(m_id) << " try_read fifo=" << raw_value(fifo));
  return make_dummy_can_message_ts();
}

result M_can::write(const CanFrameView& message) noexcept {
  (void)message;
  LOG("M_can " << toText(m_id) << " write");
  return result::OK;
}

result M_can::try_write(const CanFrameView& message) noexcept {
  (void)message;
  LOG("M_can " << toText(m_id) << " try_write");
  return result::OK;
}

result M_can::set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs)) {
  (void)callback;
  LOG("M_can " << toText(m_id) << " set_rx_callback fifo=" << raw_value(fifo));
  return result::OK;
}

result M_can::set_txfull_callback(void (*callback)()) {
  (void)callback;
  LOG("M_can " << toText(m_id) << " set_txfull_callback");
  return result::OK;
}

result M_can::set_not_matching(std::optional<M_fifo> fifo) {
  LOG("M_can " << toText(m_id) << " set_not_matching has_value="
               << raw_value(fifo.has_value()));
  return result::OK;
}

expected::expected<std::optional<M_fifo>, result> M_can::get_not_matching() {
  LOG("M_can " << toText(m_id) << " get_not_matching");
  return std::optional<M_fifo>{};
}

result M_can::reset_timestamp() {
  LOG("M_can " << toText(m_id) << " reset_timestamp");
  return result::OK;
}

expected::expected<Timestamp, result> M_can::get_timestamp() {
  LOG("M_can " << toText(m_id) << " get_timestamp");
  return static_cast<Timestamp>(0U);
}

result M_can::set_priority(M_fifo fifo, uint8_t priority) {
  LOG("M_can " << toText(m_id) << " set_priority fifo=" << raw_value(fifo)
               << " priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> M_can::get_priority(M_fifo fifo) {
  LOG("M_can " << toText(m_id) << " get_priority fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

result M_can::set_interrupt(M_fifo fifo, bool on) {
  LOG("M_can " << toText(m_id) << " set_interrupt fifo=" << raw_value(fifo)
               << " on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> M_can::is_interrupt_on(M_fifo fifo) {
  LOG("M_can " << toText(m_id) << " is_interrupt_on fifo=" << raw_value(fifo));
  return static_cast<uint8_t>(0U);
}

result M_can::set_error_priority(uint8_t priority) {
  LOG("M_can " << toText(m_id) << " set_error_priority priority=" << raw_value(priority));
  return result::OK;
}

expected::expected<uint8_t, result> M_can::get_error_priority() {
  LOG("M_can " << toText(m_id) << " get_error_priority");
  return static_cast<uint8_t>(0U);
}

result M_can::set_error_interrupt(bool on) {
  LOG("M_can " << toText(m_id) << " set_error_interrupt on=" << raw_value(on));
  return result::OK;
}

expected::expected<uint8_t, result> M_can::is_error_interrupt_on() {
  LOG("M_can " << toText(m_id) << " is_error_interrupt_on");
  return static_cast<uint8_t>(0U);
}

result M_can::set_filter(M_filter filter, uint8_t id) {
  (void)filter;
  LOG("M_can " << toText(m_id) << " set_filter id=" << raw_value(id));
  return result::OK;
}

result M_can::enable_filter(uint8_t id) {
  LOG("M_can " << toText(m_id) << " enable_filter id=" << raw_value(id));
  return result::OK;
}

result M_can::disable_filter(uint8_t id) {
  LOG("M_can " << toText(m_id) << " disable_filter id=" << raw_value(id));
  return result::OK;
}

expected::expected<bool, result> M_can::is_filter_enabled(uint8_t id) {
  LOG("M_can " << toText(m_id) << " is_filter_enabled id=" << raw_value(id));
  return false;
}

result M_canRx::set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs)) {
  return const_cast<M_can&>(m_can).set_rx_callback(fifo, callback);
}

result M_canRx::set_not_matching(std::optional<M_fifo> fifo) {
  return const_cast<M_can&>(m_can).set_not_matching(fifo);
}

expected::expected<std::optional<M_fifo>, result> M_canRx::get_not_matching() {
  return const_cast<M_can&>(m_can).get_not_matching();
}

result M_canRx::reset_timestamp() {
  return const_cast<M_can&>(m_can).reset_timestamp();
}

expected::expected<Timestamp, result> M_canRx::get_timestamp() {
  return const_cast<M_can&>(m_can).get_timestamp();
}

result M_canRx::set_priority(M_fifo fifo, uint8_t priority) {
  return const_cast<M_can&>(m_can).set_priority(fifo, priority);
}

expected::expected<uint8_t, result> M_canRx::get_priority(M_fifo fifo) {
  return const_cast<M_can&>(m_can).get_priority(fifo);
}

result M_canRx::set_interrupt(M_fifo fifo, bool on) {
  return const_cast<M_can&>(m_can).set_interrupt(fifo, on);
}

expected::expected<uint8_t, result> M_canRx::is_interrupt_on(M_fifo fifo) {
  return const_cast<M_can&>(m_can).is_interrupt_on(fifo);
}

result M_canRx::set_error_priority(uint8_t priority) {
  return const_cast<M_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> M_canRx::get_error_priority() {
  return const_cast<M_can&>(m_can).get_error_priority();
}

result M_canRx::set_error_interrupt(bool on) {
  return const_cast<M_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> M_canRx::is_error_interrupt_on() {
  return const_cast<M_can&>(m_can).is_error_interrupt_on();
}

result M_canRx::set_filter(M_filter filter, uint8_t id) {
  return const_cast<M_can&>(m_can).set_filter(filter, id);
}

result M_canRx::enable_filter(uint8_t id) {
  return const_cast<M_can&>(m_can).enable_filter(id);
}

result M_canRx::disable_filter(uint8_t id) {
  return const_cast<M_can&>(m_can).disable_filter(id);
}

expected::expected<bool, result> M_canRx::is_filter_enabled(uint8_t id) {
  return const_cast<M_can&>(m_can).is_filter_enabled(id);
}

result M_canTx::set_txfull_callback(void (*callback)()) {
  return const_cast<M_can&>(m_can).set_txfull_callback(callback);
}

result M_canTx::reset_timestamp() {
  return const_cast<M_can&>(m_can).reset_timestamp();
}

expected::expected<Timestamp, result> M_canTx::get_timestamp() {
  return const_cast<M_can&>(m_can).get_timestamp();
}

result M_canTx::set_error_priority(uint8_t priority) {
  return const_cast<M_can&>(m_can).set_error_priority(priority);
}

expected::expected<uint8_t, result> M_canTx::get_error_priority() {
  return const_cast<M_can&>(m_can).get_error_priority();
}

result M_canTx::set_error_interrupt(bool on) {
  return const_cast<M_can&>(m_can).set_error_interrupt(on);
}

expected::expected<uint8_t, result> M_canTx::is_error_interrupt_on() {
  return const_cast<M_can&>(m_can).is_error_interrupt_on();
}
}  // namespace ru::driver
