#include "serial.hpp"
#include "mapping.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
constexpr opaque_serial make_opaque(const SerialId id) noexcept {
  switch (id) {
#define RU_POSIX_SERIAL_CASE(name, device_path)                                            \
    case SerialId::name:                                                                   \
      return opaque_serial{device_path};
    RU_POSIX_SERIAL_MAP(RU_POSIX_SERIAL_CASE)
#undef RU_POSIX_SERIAL_CASE
    default:
      return {};
  }
}
}  // namespace

Serial::Serial() noexcept : m_id(SerialId::USB), m_opaque(make_opaque(m_id)) {
}

result Serial::start() noexcept {
  return result::OK;
}

result Serial::init() noexcept {
  return m_opaque.init();
}

result Serial::stop() noexcept {
  return m_opaque.stop();
}

result Serial::write(const uint8_t* const p_data, const size_t len,
                     const Timestamp timeout_uS) noexcept {
  if (p_data == nullptr && len != 0U) {
    return result::RECOVERABLE_ERROR;
  }

  return m_opaque.write(p_data, len, timeout_uS);
}

result Serial::read(uint8_t* const p_data, const size_t len,
                    const Timestamp timeout_uS) noexcept {
  if (p_data == nullptr && len != 0U) {
    return result::RECOVERABLE_ERROR;
  }

  if (len == 0U) {
    return result::OK;
  }

  return m_opaque.read(p_data, len, timeout_uS);
}
}  // namespace ru::driver
