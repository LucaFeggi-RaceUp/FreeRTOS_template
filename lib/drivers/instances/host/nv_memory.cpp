#include "nv_memory.hpp"

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

opaque_nv_memory make_opaque(const NvMemoryId id) noexcept {
  (void)id;
  return opaque_nv_memory{};
}
}  // namespace

opaque_nv_memory::opaque_nv_memory(const uint32_t capacity) noexcept
    : m_capacity(capacity <= k_storage_capacity ? capacity : k_storage_capacity) {
  m_storage.fill(0xFFU);
}

result opaque_nv_memory::init() noexcept {
  m_initialized = true;
  return result::OK;
}

result opaque_nv_memory::stop() noexcept {
  m_initialized = false;
  return result::OK;
}

uint32_t opaque_nv_memory::capacity() const noexcept {
  return m_capacity;
}

result opaque_nv_memory::clear() noexcept {
  if (!m_initialized) {
    return result::RECOVERABLE_ERROR;
  }

  std::fill_n(m_storage.begin(), m_capacity, 0xFFU);
  return result::OK;
}

result opaque_nv_memory::read(const uint32_t address, uint8_t* const p_data,
                              const size_t len) const noexcept {
  if (!m_initialized || !range_valid(m_capacity, address, len) ||
      (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  for (size_t index = 0U; index < len; ++index) {
    p_data[index] = m_storage[address + index];
  }

  return result::OK;
}

result opaque_nv_memory::write(const uint32_t address, const uint8_t* const p_data,
                               const size_t len) noexcept {
  if (!m_initialized || !range_valid(m_capacity, address, len) ||
      (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  for (size_t index = 0U; index < len; ++index) {
    m_storage[address + index] = p_data[index];
  }

  return result::OK;
}

NvMemory::NvMemory(const NvMemoryId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
  LOG("NvMemory " << toText(m_id) << " create");
}

result NvMemory::start() noexcept {
  LOG("NvMemory driver start");
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
  return m_opaque.clear();
}

result NvMemory::read(const uint32_t address, uint8_t* const p_data,
                      const size_t len) noexcept {
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
  return m_opaque.write(address, p_data, len);
}

result NvMemory::write(const uint32_t address, const uint8_t value) noexcept {
  return write(address, &value, sizeof(value));
}
}  // namespace ru::driver
