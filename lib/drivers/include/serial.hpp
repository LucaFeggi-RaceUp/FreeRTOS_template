#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "common.hpp"
#include "opaque_serial.hpp"
#include "serial_id.hpp"

namespace ru::driver {

class Serial {
 public:
  Serial() noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  result write(const uint8_t* data, size_t len, Timestamp timeout_uS) noexcept;
  result read(uint8_t* data, size_t len, Timestamp timeout_uS) noexcept;

  SerialId inline id() const noexcept { return m_id; }

 private:
  SerialId m_id;
  struct opaque_serial m_opaque;
};

}  // namespace ru::driver
