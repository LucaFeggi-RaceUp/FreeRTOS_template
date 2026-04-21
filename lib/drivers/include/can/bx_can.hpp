#pragma once

#include <cstdint>
#include <variant>

#include "can_id.hpp"
#include "can/can.hpp"
#include "opaque_can.hpp"

namespace ru::driver {

class Bx_canRx;
class Bx_canTx;

enum class Bx_fifo : uint8_t { FIFO0, FIFO1 };

struct Bx_Mask32 {
  uint32_t id;
  uint32_t mask;
  CanIdFormat format{CanIdFormat::Standard};
};
struct Bx_List32 {
  uint32_t id1;
  uint32_t id2;
  CanIdFormat format{CanIdFormat::Standard};
};
struct Bx_Mask16 {
  uint16_t id1;
  uint16_t mask1;
  uint16_t id2;
  uint16_t mask2;
};
struct Bx_List16 {
  uint16_t id1;
  uint16_t id2;
  uint16_t id3;
  uint16_t id4;
};

using Bx_FilterConfig = std::variant<Bx_Mask32, Bx_List32, Bx_Mask16, Bx_List16>;

struct Bx_filter {
  Bx_fifo fifo;
  Bx_FilterConfig config;
};

class Bx_can {
 public:
  Bx_canTx inline into_tx() & noexcept;
  Bx_canRx inline into_rx() & noexcept;

  Bx_can(Bx_canId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  expected::expected<CanMessage, result> read(Bx_fifo fifo) noexcept;
  expected::expected<CanMessage, result> try_read(Bx_fifo fifo) noexcept;
  result write(const CanMessage& message) noexcept;
  result try_write(const CanMessage& message) noexcept;

  Bx_canId inline id() const noexcept { return m_id; }

  result set_priority(Bx_fifo fifo, uint8_t priority);
  expected::expected<uint8_t, result> get_priority(Bx_fifo fifo);

  result set_interrupt(Bx_fifo fifo, bool on);
  expected::expected<uint8_t, result> is_interrupt_on(Bx_fifo fifo);

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Bx_filter filter,uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Bx_canId m_id;
  struct opaque_can m_opaque;
};

class Bx_canRx {
 public:
  explicit inline Bx_canRx(Bx_can& can) noexcept : m_can(can) {}
  expected::expected<CanMessage, result> inline read(Bx_fifo fifo) noexcept;
  expected::expected<CanMessage, result> inline try_read(Bx_fifo fifo) noexcept;

  Bx_canId inline id() const noexcept { return m_can.id(); }

  result set_priority(Bx_fifo fifo, uint8_t priority);
  expected::expected<uint8_t, result> get_priority(Bx_fifo fifo);

  result set_interrupt(Bx_fifo fifo, bool on);
  expected::expected<uint8_t, result> is_interrupt_on(Bx_fifo fifo);

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

  result set_filter(Bx_filter filter, uint8_t id);
  result enable_filter(uint8_t id);
  result disable_filter(uint8_t id);
  expected::expected<bool, result> is_filter_enabled(uint8_t id);

 private:
  const Bx_can& m_can;
};

class Bx_canTx {
 public:
  explicit inline Bx_canTx(Bx_can& can) noexcept : m_can(can) {}
  result inline write(const CanMessage& message) noexcept;
  result inline try_write(const CanMessage& message) noexcept;

  Bx_canId inline id() const noexcept { return m_can.id(); }

  result set_error_priority(uint8_t priority);
  expected::expected<uint8_t, result> get_error_priority();

  result set_error_interrupt(bool on);
  expected::expected<uint8_t, result> is_error_interrupt_on();

 private:
  const Bx_can& m_can;
};

inline Bx_canTx Bx_can::into_tx() & noexcept { return Bx_canTx(*this); }

inline Bx_canRx Bx_can::into_rx() & noexcept { return Bx_canRx(*this); }

}  // namespace ru::driver
