#include "opaque_serial.hpp"

#include "utils/common.hpp"
#include "usb/usb_cdc.h"

using namespace ru::driver;

namespace ru::driver {
result opaque_serial::init() const noexcept {
  return ru_stm32_usb_cdc_init() == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_serial::stop() const noexcept {
  return ru_stm32_usb_cdc_stop() == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_serial::write(const uint8_t* const p_data, const size_t len,
                            const Timestamp timeout_uS) const noexcept {
  return ru_stm32_usb_cdc_write(p_data, len, timeout_to_ms(timeout_uS)) == 0 ? result::OK
                                                                              : result::RECOVERABLE_ERROR;
}

result opaque_serial::read(uint8_t* const p_data, const size_t len,
                           const Timestamp timeout_uS) const noexcept {
  return ru_stm32_usb_cdc_read(p_data, len, timeout_to_ms(timeout_uS)) == 0 ? result::OK
                                                                             : result::RECOVERABLE_ERROR;
}
}  // namespace ru::driver
