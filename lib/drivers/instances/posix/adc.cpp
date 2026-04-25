#include "adc.hpp"

#include <algorithm>
#include <cmath>

#include "common.hpp"
#include "mapping.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
uint16_t to_adc_counts(const double value) noexcept {
  const auto clamped = std::clamp(value, 0.0, 3.3);
  return static_cast<uint16_t>(std::lround((clamped / 3.3) * 4095.0));
}

result parse_adc_value(const virtual_driver::Result& r_result, uint16_t& r_value) noexcept {
  double value{0.0};
  if (!parse_generated_double(r_result, value)) {
    return result::RECOVERABLE_ERROR;
  }

  r_value = to_adc_counts(value);
  return result::OK;
}

#define RU_POSIX_DEFINE_ADC_READ(wrapper_name, api_name)                                \
  result wrapper_name##_read(uint16_t& r_value) noexcept {                              \
    return parse_adc_value(generated_api().api_name##_read(), r_value);                 \
  }

RU_POSIX_DEFINE_ADC_READ(adc0, adc_adc_0)
RU_POSIX_DEFINE_ADC_READ(adc1, adc_adc_1)
RU_POSIX_DEFINE_ADC_READ(adc2, adc_adc_2)
RU_POSIX_DEFINE_ADC_READ(adc3, adc_adc_3)

#undef RU_POSIX_DEFINE_ADC_READ

constexpr opaque_adc make_opaque(const AdcId id) noexcept {
  switch (id) {
#define RU_POSIX_ADC_CASE(enum_name, wrapper_name)                                        \
    case AdcId::enum_name:                                                                \
      return opaque_adc{wrapper_name##_read};
    RU_POSIX_ADC_MAP(RU_POSIX_ADC_CASE)
#undef RU_POSIX_ADC_CASE
    default:
      return {};
  }
}
}  // namespace

result Adc::start() noexcept {
  return result::OK;
}

Adc::Adc(const AdcId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
}

result Adc::init() noexcept {
  return result::OK;
}

result Adc::stop() noexcept {
  return result::OK;
}

expected::expected<uint16_t, result> Adc::read() noexcept {
  uint16_t value{0U};
  const auto status = m_opaque.read(value);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return value;
}

expected::expected<std::optional<uint16_t>, result> Adc::try_read() noexcept {
  const auto value = read();
  if (!value.has_value()) {
    return expected::unexpected(value.error());
  }

  return std::optional<uint16_t>{value.value()};
}
}  // namespace ru::driver
