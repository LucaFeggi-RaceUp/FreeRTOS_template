#include "opaque_nv_memory.hpp"

namespace ru::driver {
namespace {
bool range_valid(const uint32_t capacity, const uint32_t address,
                 const size_t len) noexcept {
  return static_cast<uint64_t>(address) + static_cast<uint64_t>(len) <=
         static_cast<uint64_t>(capacity);
}
}  // namespace

result opaque_nv_memory::init() noexcept {
  if (m_capacity == 0U) {
    return result::UNRECOVERABLE_ERROR;
  }

  switch (m_backend) {
    case NvMemoryBackend::EmulatedEeprom:
      return m_eeprom.init();

    case NvMemoryBackend::FlashRegion: {
      const auto status = m_flash.init();
      if (status != result::OK) {
        return status;
      }

      const auto flash_capacity = m_flash.capacity();
      return static_cast<uint64_t>(m_flash_offset) +
                     static_cast<uint64_t>(m_capacity) <=
                 static_cast<uint64_t>(flash_capacity)
             ? result::OK
             : result::UNRECOVERABLE_ERROR;
    }

    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result opaque_nv_memory::stop() noexcept {
  switch (m_backend) {
    case NvMemoryBackend::EmulatedEeprom:
      return m_eeprom.stop();

    case NvMemoryBackend::FlashRegion:
      return m_flash.stop();

    default:
      return result::OK;
  }
}

result opaque_nv_memory::clear() noexcept {
  switch (m_backend) {
    case NvMemoryBackend::EmulatedEeprom:
      return m_eeprom.clear();

    case NvMemoryBackend::FlashRegion:
      return m_flash.erase(m_flash_offset, m_capacity);

    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result opaque_nv_memory::read(const uint32_t address, uint8_t* const p_data,
                              const size_t len) const noexcept {
  if (!range_valid(m_capacity, address, len) || (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  switch (m_backend) {
    case NvMemoryBackend::EmulatedEeprom:
      return m_eeprom.read(address, p_data, len);

    case NvMemoryBackend::FlashRegion:
      return m_flash.read(m_flash_offset + address, p_data, len);

    default:
      return result::UNRECOVERABLE_ERROR;
  }
}

result opaque_nv_memory::write(const uint32_t address, const uint8_t* const p_data,
                               const size_t len) noexcept {
  if (!range_valid(m_capacity, address, len) || (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  switch (m_backend) {
    case NvMemoryBackend::EmulatedEeprom:
      return m_eeprom.write(address, p_data, len);

    case NvMemoryBackend::FlashRegion:
      return m_flash.write(m_flash_offset + address, p_data, len);

    default:
      return result::UNRECOVERABLE_ERROR;
  }
}
}  // namespace ru::driver
