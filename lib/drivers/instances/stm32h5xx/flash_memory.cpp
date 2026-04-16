#include "flash_memory.hpp"

using namespace ru::driver;

namespace ru::driver {
FlashMemory::FlashMemory(const FlashMemoryId id) noexcept : m_id(id), m_opaque() {
}

result FlashMemory::start() noexcept {
  return result::OK;
}

result FlashMemory::init() noexcept {
  return m_opaque.init();
}

result FlashMemory::stop() noexcept {
  return m_opaque.stop();
}

result FlashMemory::read(const uint32_t addr, uint8_t* const p_data,
                         const size_t len) noexcept {
  return m_opaque.read(addr, p_data, len);
}

result FlashMemory::write(const uint32_t addr, const uint8_t* const p_data,
                          const size_t len) noexcept {
  return m_opaque.write(addr, p_data, len);
}

result FlashMemory::erase(const uint32_t addr, const size_t len) noexcept {
  return m_opaque.erase(addr, len);
}

result FlashMemory::erase_all() noexcept {
  return m_opaque.erase_all();
}
}  // namespace ru::driver
