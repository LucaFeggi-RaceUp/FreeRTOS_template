#include "eeprom.hpp"

#include "mapping.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
opaque_eeprom make_opaque(const EepromId id) noexcept {
  switch (id) {
#define RU_STM32H5XX_EEPROM_CASE(name, offset, capacity) \
    case EepromId::name:                                  \
      return opaque_eeprom{offset, capacity};
    RU_STM32H5XX_EEPROM_MAP(RU_STM32H5XX_EEPROM_CASE)
#undef RU_STM32H5XX_EEPROM_CASE
    default:
      return opaque_eeprom{};
  }
}
}  // namespace

Eeprom::Eeprom(const EepromId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
}

result Eeprom::start() noexcept {
  return result::OK;
}

result Eeprom::init() noexcept {
  return m_opaque.init();
}

result Eeprom::stop() noexcept {
  return m_opaque.stop();
}

uint32_t Eeprom::capacity() const noexcept {
  return m_opaque.capacity();
}

result Eeprom::read(const uint32_t address, uint8_t* const p_data,
                    const size_t len) noexcept {
  return m_opaque.read(address, p_data, len);
}

expected::expected<uint8_t, result> Eeprom::read(const uint32_t address) noexcept {
  uint8_t value{0U};
  const auto status = read(address, &value, sizeof(value));
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return value;
}

result Eeprom::write(const uint32_t address, const uint8_t* const p_data,
                     const size_t len) noexcept {
  return m_opaque.write(address, p_data, len);
}

result Eeprom::write(const uint32_t address, const uint8_t value) noexcept {
  return write(address, &value, sizeof(value));
}
}  // namespace ru::driver
