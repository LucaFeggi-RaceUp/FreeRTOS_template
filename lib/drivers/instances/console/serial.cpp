#include "serial.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
Serial::Serial() noexcept : m_id(SerialId::USB) {
  LOG("Serial " << toText(m_id) << " create");
}

result Serial::start() noexcept {
  LOG("Serial driver start");
  return result::OK;
}

result Serial::init() noexcept {
  LOG("Serial " << toText(m_id) << " init");
  return result::OK;
}

result Serial::stop() noexcept {
  LOG("Serial " << toText(m_id) << " stop");
  return result::OK;
}

result Serial::write(const uint8_t* const p_data, const size_t len,
                     const Timestamp timeout_uS) noexcept {
  LOG("Serial " << toText(m_id) << " write");
  return result::OK;
}

result Serial::read(uint8_t* const p_data, const size_t len,
                    const Timestamp timeout_uS) noexcept {
  LOG("Serial " << toText(m_id) << " read");
  return result::OK;
}
}  // namespace ru::driver
