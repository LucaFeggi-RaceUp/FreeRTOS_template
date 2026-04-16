#include "serial.hpp"

#include <cstdio>

using namespace ru::driver;

namespace ru::driver {
Serial::Serial() noexcept : m_id(SerialId::USB) {
}

result Serial::start() noexcept {
  return result::OK;
}

result Serial::init() noexcept {
  return result::OK;
}

result Serial::stop() noexcept {
  return result::OK;
}

result Serial::write(const uint8_t* const p_data, const size_t len,
                     const Timestamp timeout_uS) noexcept {
  (void)timeout_uS;

  if (p_data == nullptr && len != 0U) {
    return result::RECOVERABLE_ERROR;
  }

  if (len == 0U) {
    return result::OK;
  }

  const auto written = std::fwrite(p_data, sizeof(uint8_t), len, stdout);
  std::fflush(stdout);
  return written == len ? result::OK : result::RECOVERABLE_ERROR;
}

result Serial::read(uint8_t* const p_data, const size_t len,
                    const Timestamp timeout_uS) noexcept {
  (void)timeout_uS;

  if (p_data == nullptr && len != 0U) {
    return result::RECOVERABLE_ERROR;
  }

  if (len == 0U) {
    return result::OK;
  }

  const auto read = std::fread(p_data, sizeof(uint8_t), len, stdin);
  return read == len ? result::OK : result::RECOVERABLE_ERROR;
}
}  // namespace ru::driver
