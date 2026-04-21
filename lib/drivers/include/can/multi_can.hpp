#pragma once

#include <cstdint>
#include <optional>
#include <variant>

#include "can/can.hpp"
#include "can_id.hpp"
#include "opaque_can.hpp"

namespace ru::driver {

class CanRx;
class CanTx;

enum class Multi_target : uint8_t {
  STORE_IN_BUFFER_BLOCKING,
  STORE_IN_BUFFER_OVERWRITE,
  STORE_FIFO_0,
  STORE_FIFO_1,
  REJECT
};

struct Multi_Mask {
  uint32_t id;
  uint32_t mask;
  CanIdFormat format{CanIdFormat::Standard};
};
struct Multi_Dual {
  uint32_t id1;
  uint32_t id2;
  CanIdFormat format{CanIdFormat::Standard};
};
struct Multi_Range {
  uint32_t from;
  uint32_t to;
  CanIdFormat format{CanIdFormat::Standard};
};

using Multi_FilterConfig =
    std::optional<std::variant<Multi_Mask, Multi_Dual, Multi_Range>>;

struct Multi_filter {
  enum Multi_target target;
  Multi_FilterConfig config;
};

enum class Multi_fifo : uint8_t { FIFO_0, FIFO_1 };

class Can {
 public:
  CanTx inline into_tx() & noexcept;
  CanRx inline into_rx() & noexcept;

  Can(Multi_canId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  Multi_canId inline id() const noexcept { return m_id; }

  expected::expected<CanMessageTs, result> read(uint8_t buffer) noexcept;
  expected::expected<CanMessageTs, result> read(Multi_fifo fifo) noexcept;
  expected::expected<CanMessageTs, result> try_read(uint8_t buffer) noexcept;
  expected::expected<CanMessageTs, result> try_read(Multi_fifo fifo) noexcept;

  result write(const CanMessage message) noexcept;
  result try_write(const CanMessage message) noexcept;

  result set_rx_callback(Multi_fifo fifo, void (*callback)(CanMessageTs));
  result set_rx_callback(uint8_t buffer, void (*callback)(CanMessageTs));
  result enable_rx_interrupt(Multi_fifo fifo);
  result enable_rx_interrupt(uint8_t buffer);
  result disable_rx_interrupt(Multi_fifo fifo);
  result disable_rx_interrupt(uint8_t buffer);

  expected::expected<Timestamp, result> get_timestamp();

  result set_rx_priority(Multi_fifo fifo, uint8_t priority);
  result set_rx_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_rx_priority();
  expected::expected<uint8_t, result> get_rx_priority(Multi_fifo fifo);

  result set_rx_interrupt(Multi_fifo fifo, bool on);
  result set_rx_interrupt(bool on);
  expected::expected<uint8_t, result> is_rx_interrupt_on(Multi_fifo fifo);
  expected::expected<uint8_t, result> is_rx_interrupt_on();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Multi_filter filter, uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Multi_canId m_id;
  struct opaque_can m_opaque;
};

class CanRx {
 public:
  explicit inline CanRx(Can& can) noexcept : m_can(can) {}
  Multi_canId inline id() const noexcept { return m_can.id(); }

  expected::expected<CanMessageTs, result> read(uint8_t buffer) noexcept;
  expected::expected<CanMessageTs, result> read(Multi_fifo fifo) noexcept;
  expected::expected<CanMessageTs, result> try_read(uint8_t buffer) noexcept;
  expected::expected<CanMessageTs, result> try_read(Multi_fifo fifo) noexcept;

  result set_rx_callback(Multi_fifo fifo, void (*callback)(CanMessageTs));
  result set_rx_callback(uint8_t buffer, void (*callback)(CanMessageTs));
  result enable_rx_interrupt(Multi_fifo fifo);
  result enable_rx_interrupt(uint8_t buffer);
  result disable_rx_interrupt(Multi_fifo fifo);
  result disable_rx_interrupt(uint8_t buffer);

  expected::expected<Timestamp, result> get_timestamp();

  result set_rx_priority(Multi_fifo fifo, uint8_t priority);
  result set_rx_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_rx_priority();
  expected::expected<uint8_t, result> get_rx_priority(Multi_fifo fifo);

  result set_rx_interrupt(Multi_fifo fifo, bool on);
  result set_rx_interrupt(bool on);
  expected::expected<uint8_t, result> is_rx_interrupt_on(Multi_fifo fifo);
  expected::expected<uint8_t, result> is_rx_interrupt_on();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Multi_filter filter, uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Can& m_can;
};

class CanTx {
 public:
  explicit inline CanTx(Can& can) noexcept : m_can(can) {}
  Multi_canId inline id() const noexcept { return m_can.id(); }

  result write(const CanMessage message) noexcept;
  result try_write(const CanMessage message) noexcept;

  expected::expected<Timestamp, result> get_timestamp();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

 private:
  const Can& m_can;
};

inline CanTx Can::into_tx() & noexcept { return CanTx(*this); }

inline CanRx Can::into_rx() & noexcept { return CanRx(*this); }

}  // namespace ru::driver
