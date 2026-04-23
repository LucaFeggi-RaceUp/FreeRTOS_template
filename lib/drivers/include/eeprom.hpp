#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"
#include "eeprom_id.hpp"
#include "opaque_eeprom.hpp"

namespace ru::driver {

class Eeprom {
 public:
  Eeprom(EepromId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  uint32_t capacity() const noexcept;
  result clear() noexcept;
  result read(uint32_t address, uint8_t* data, size_t len) noexcept;
  expected::expected<uint8_t, result> read(uint32_t address) noexcept;
  result write(uint32_t address, const uint8_t* data, size_t len) noexcept;
  result write(uint32_t address, uint8_t value) noexcept;

  EepromId inline id() const noexcept { return m_id; }

 private:
  EepromId m_id;
  struct opaque_eeprom m_opaque;
};

}  // namespace ru::driver
