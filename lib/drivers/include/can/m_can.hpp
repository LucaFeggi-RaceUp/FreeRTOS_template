#pragma once

#include <cstdint>
#include <optional>
#include <variant>

#include "can/can.hpp"
#include "can_id.hpp"
#include "opaque_can.hpp"

namespace ru::driver {

class M_canRx;
class M_canTx;

enum class M_fifo : uint8_t { FIFO0, FIFO1 };

struct M_Mask {
  uint32_t id;
  uint32_t mask;
};
struct M_Dual {
  uint32_t id1;
  uint32_t id2;
};
struct M_Range {
  uint32_t from;
  uint32_t to;
};

using M_FilterConfig = std::variant<M_Mask, M_Dual, M_Range>;

struct M_filter {
  M_fifo fifo;
  M_FilterConfig config;
};

class M_can {
 public:
  M_canTx inline into_tx() & noexcept;
  M_canRx inline into_rx() & noexcept;

  M_can(M_canId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  expected::expected<CanMessageTs, result> read(M_fifo fifo) noexcept;
  expected::expected<CanMessageTs, result> try_read(M_fifo fifo) noexcept;
  result write(const CanMessage& message) noexcept;
  result try_write(const CanMessage& message) noexcept;

  M_canId inline id() const noexcept { return m_id; }

  result set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs));
  result set_txfull_callback(void (*callback)());

  result set_not_matching(std::optional<M_fifo> fifo);
  expected::expected<std::optional<M_fifo>, result> get_not_matching();

  result reset_timestamp();
  expected::expected<Timestamp, result> get_timestamp();

  result set_priority(M_fifo fifo, uint8_t priority);
  expected::expected<uint8_t, result> get_priority(M_fifo fifo);

  result set_interrupt(M_fifo fifo, bool on);
  expected::expected<uint8_t, result> is_interrupt_on(M_fifo fifo);

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(M_filter filter, uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const M_canId m_id;
  struct opaque_can m_opaque;
};

class M_canRx {
 public:
  explicit inline M_canRx(M_can& can) noexcept : m_can(can) {}
  expected::expected<CanMessageTs, result> inline read(M_fifo fifo) noexcept;
  expected::expected<CanMessageTs, result> inline try_read(M_fifo fifo) noexcept;

  M_canId inline id() const noexcept { return m_can.id(); }

  result set_rx_callback(M_fifo fifo, void (*callback)(CanMessageTs));

  result set_not_matching(std::optional<M_fifo> fifo);
  expected::expected<std::optional<M_fifo>, result> get_not_matching();

  result reset_timestamp();
  expected::expected<Timestamp, result> get_timestamp();

  result set_priority(M_fifo fifo, uint8_t priority);
  expected::expected<uint8_t, result> get_priority(M_fifo fifo);

  result set_interrupt(M_fifo fifo, bool on);
  expected::expected<uint8_t, result> is_interrupt_on(M_fifo fifo);

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(M_filter filter, uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const M_can& m_can;
};

class M_canTx {
 public:
  explicit inline M_canTx(M_can& can) noexcept : m_can(can) {}
  result inline write(const CanMessage& message) noexcept;
  result inline try_write(const CanMessage& message) noexcept;

  M_canId inline id() const noexcept { return m_can.id(); }

  result set_txfull_callback(void (*callback)());

  result reset_timestamp();
  expected::expected<Timestamp, result> get_timestamp();

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

 private:
  const M_can& m_can;
};

inline M_canTx M_can::into_tx() & noexcept { return M_canTx(*this); }

inline M_canRx M_can::into_rx() & noexcept { return M_canRx(*this); }

}  // namespace ru::driver
