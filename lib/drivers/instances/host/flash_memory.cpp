#include "flash_memory.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
FlashMemory::FlashMemory(const FlashMemoryId id) noexcept : m_id(id) {
  LOG("FlashMemory " << toText(m_id) << " create");
}

result FlashMemory::start() noexcept {
  LOG("FlashMemory driver started");
  return result::OK;
}

result FlashMemory::init() noexcept {
  LOG("FlashMemory " << toText(m_id) << " init");
  return result::OK;
}

result FlashMemory::stop() noexcept {
  LOG("FlashMemory " << toText(m_id) << " stop");
  return result::OK;
}

result FlashMemory::read(const uint32_t addr, uint8_t* const p_data,
                         const size_t len) noexcept {
  LOG("FlashMemory" << toText(m_id) << "read");
  return result::OK;
}

result FlashMemory::write(const uint32_t addr, const uint8_t* const p_data,
                          const size_t len) noexcept {
  LOG("FlashMemory " << toText(m_id) << " write");
  return result::OK;
}

result FlashMemory::erase(const uint32_t addr, const size_t len) noexcept {
  LOG("FlashMemory " << toText(m_id) << " erase");
  return result::OK;
}

result FlashMemory::erase_all() noexcept {
  LOG("FlashMemory " << toText(m_id) << " erase_all");
  return result::OK;
}

}  // namespace ru::driver
