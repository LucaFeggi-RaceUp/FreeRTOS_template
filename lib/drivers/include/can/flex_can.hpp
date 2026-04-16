// Might be reworked once we deploy it for the first time do do not implement

#pragma once

#include <cstdint>

#include "can/can.hpp"
#include "can_id.hpp"
#include "opaque_can.hpp"

namespace ru::driver {

class Flex_canRx;
class Flex_canTx;

struct Flex_Mask {
  uint32_t id;
  uint32_t mask;
};

struct Flex_filter {
  uint8_t MB;
  Flex_Mask config;
};

class Flex_can {
 public:
  Flex_canTx inline into_tx() & noexcept;
  Flex_canRx inline into_rx() & noexcept;

  Flex_can(Flex_canId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  expected::expected<CanMessageTs, result> read() noexcept;
  expected::expected<CanMessageTs, result> read(uint8_t mb) noexcept;
  expected::expected<CanMessageTs, result> try_read() noexcept;
  expected::expected<CanMessageTs, result> try_read(uint8_t mb) noexcept;
  result write(const CanMessage& message) noexcept;
  result try_write(const CanMessage& message) noexcept;

  Flex_canId inline id() const noexcept { return m_id; }

  result set_rx_callback(void (*callback)(CanMessageTs));
  result enable_rx_interrupt(uint8_t id);
  result disable_rx_interrupt(uint8_t id);

  expected::expected<Timestamp, result> get_timestamp();

  result set_rx_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_rx_priority();

  result set_rx_interrupt(bool on);
  expected::expected<uint8_t, result> is_rx_interrupt_on();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Flex_filter filter,uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

  result set_fifo_mask(uint16_t* Ids, uint8_t len);

 private:
  const Flex_canId m_id;
  struct opaque_can m_opaque;
};

class Flex_canRx {
 public:
  explicit inline Flex_canRx(Flex_can& can) noexcept : m_can(can) {}
  Flex_canId inline id() const noexcept { return m_can.id(); }
  expected::expected<CanMessageTs, result> read() noexcept;
  expected::expected<CanMessageTs, result> read(uint8_t mb) noexcept;
  expected::expected<CanMessageTs, result> try_read() noexcept;
  expected::expected<CanMessageTs, result> try_read(uint8_t mb) noexcept;

  result set_rx_callback(void (*callback)(CanMessageTs));
  result enable_rx_interrupt(uint8_t id);
  result disable_rx_interrupt(uint8_t id);

  expected::expected<Timestamp, result> get_timestamp();

  result set_rx_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_rx_priority();

  result set_rx_interrupt(bool on);
  expected::expected<uint8_t, result> is_rx_interrupt_on();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Flex_filter filter);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Flex_can& m_can;
};

class Flex_canTx {
 public:
  explicit inline Flex_canTx(Flex_can& can) noexcept : m_can(can) {}
  Flex_canId inline id() const noexcept { return m_can.id(); }
  result write(const CanMessage& message) noexcept;
  result try_write(const CanMessage& message) noexcept;

  expected::expected<Timestamp, result> get_timestamp();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Flex_filter filter);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Flex_can& m_can;
};

inline Flex_canTx Flex_can::into_tx() & noexcept { return Flex_canTx(*this); }

inline Flex_canRx Flex_can::into_rx() & noexcept { return Flex_canRx(*this); }

}  // namespace ru::driver
