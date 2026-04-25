#include "nv_memory.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
constexpr uint32_t capacity_for(const NvMemoryId id) noexcept {
  (void)id;
  return 0U;
}
}  // namespace

NvMemory::NvMemory(const NvMemoryId id) noexcept
    : m_id(id), m_opaque(capacity_for(id)) {
  LOG("NvMemory " << toText(m_id) << " create");
}

result NvMemory::start() noexcept {
  LOG("NvMemory driver started");
  return result::OK;
}

result NvMemory::init() noexcept {
  LOG("NvMemory " << toText(m_id) << " init");
  return m_opaque.init();
}

result NvMemory::stop() noexcept {
  LOG("NvMemory " << toText(m_id) << " stop");
  return m_opaque.stop();
}

uint32_t NvMemory::capacity() const noexcept {
  return m_opaque.capacity();
}

result NvMemory::clear() noexcept {
  LOG("NvMemory " << toText(m_id) << " clear");
  return m_opaque.clear();
}

result NvMemory::read(const uint32_t address, uint8_t* const p_data,
                      const size_t len) noexcept {
  LOG("NvMemory " << toText(m_id) << " read");
  return m_opaque.read(address, p_data, len);
}

expected::expected<uint8_t, result> NvMemory::read(const uint32_t address) noexcept {
  uint8_t value{0U};
  const auto status = read(address, &value, sizeof(value));
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return value;
}

result NvMemory::write(const uint32_t address, const uint8_t* const p_data,
                       const size_t len) noexcept {
  LOG("NvMemory " << toText(m_id) << " write");
  return m_opaque.write(address, p_data, len);
}

result NvMemory::write(const uint32_t address, const uint8_t value) noexcept {
  return write(address, &value, sizeof(value));
}
}  // namespace ru::driver
