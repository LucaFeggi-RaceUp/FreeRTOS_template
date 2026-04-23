#include "eeprom.hpp"

#include <algorithm>

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
bool range_valid(const uint32_t capacity, const uint32_t address,
                 const size_t len) noexcept {
  return static_cast<uint64_t>(address) + static_cast<uint64_t>(len) <=
         static_cast<uint64_t>(capacity);
}
}  // namespace

Eeprom::Eeprom(const EepromId id) noexcept : m_id(id), m_opaque() {
  LOG("Eeprom " << toText(m_id) << " create");
}

result Eeprom::start() noexcept {
  LOG("Eeprom driver start");
  return result::OK;
}

result Eeprom::init() noexcept {
  LOG("Eeprom " << toText(m_id) << " init");
  m_opaque.initialized = true;
  return result::OK;
}

result Eeprom::stop() noexcept {
  LOG("Eeprom " << toText(m_id) << " stop");
  m_opaque.initialized = false;
  return result::OK;
}

uint32_t Eeprom::capacity() const noexcept {
  return static_cast<uint32_t>(m_opaque.storage.size());
}

result Eeprom::clear() noexcept {
  if (!m_opaque.initialized) {
    return result::RECOVERABLE_ERROR;
  }

  std::fill(m_opaque.storage.begin(), m_opaque.storage.end(), 0xFFU);
  return result::OK;
}

result Eeprom::read(const uint32_t address, uint8_t* const p_data,
                    const size_t len) noexcept {
  if (!m_opaque.initialized || !range_valid(capacity(), address, len) ||
      (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  for (size_t index = 0U; index < len; ++index) {
    p_data[index] = m_opaque.storage[address + index];
  }

  return result::OK;
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
  if (!m_opaque.initialized || !range_valid(capacity(), address, len) ||
      (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  for (size_t index = 0U; index < len; ++index) {
    m_opaque.storage[address + index] = p_data[index];
  }

  return result::OK;
}

result Eeprom::write(const uint32_t address, const uint8_t value) noexcept {
  return write(address, &value, sizeof(value));
}
}  // namespace ru::driver
