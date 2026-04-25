#include "adc.hpp"

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {

result Adc::start() noexcept {
  LOG("Adc driver started");
  return result::OK;
}

Adc::Adc(const AdcId id) noexcept : m_id(id) {
  LOG("Adc " << toText(m_id) << " create");
}

result Adc::init() noexcept {
  LOG("Adc " << toText(m_id) << " init");
  return result::OK;
}

result Adc::stop() noexcept {
  LOG("Adc " << toText(m_id) << " stop");
  return result::OK;
}

expected::expected<uint16_t, result> Adc::read() noexcept {
  LOG("Adc " << toText(m_id) << " read");
  return 0;
}

expected::expected<std::optional<uint16_t>, result> Adc::try_read() noexcept {
  LOG("Adc " << toText(m_id) << " try_read");
  return 0;
}
}  // namespace ru::driver
