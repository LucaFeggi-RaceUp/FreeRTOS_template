#include "pwm.hpp"

#include <algorithm>
#include <cmath>

#include "common.hpp"
#include "mapping.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
double to_generated_duty(const uint16_t duty_cycle_permille) noexcept {
  return static_cast<double>(std::min<uint16_t>(duty_cycle_permille, 1000U)) / 1000.0;
}

uint16_t from_generated_duty(const double duty) noexcept {
  const auto clamped = std::clamp(duty, 0.0, 1.0);
  return static_cast<uint16_t>(std::lround(clamped * 1000.0));
}

result parse_pwm_value(const virtual_driver::Result& r_result, uint16_t& r_value) noexcept {
  double value{0.0};
  if (!parse_generated_double(r_result, value)) {
    return result::RECOVERABLE_ERROR;
  }

  r_value = from_generated_duty(value);
  return result::OK;
}

#define RU_POSIX_DEFINE_PWM_OPS(wrapper_name, api_name)                                  \
  result wrapper_name##_read(uint16_t& r_value) noexcept {                               \
    return parse_pwm_value(generated_api().api_name##_read(), r_value);                  \
  }                                                                                      \
                                                                                         \
  result wrapper_name##_write(const uint16_t duty_cycle_permille) noexcept {             \
    return result_from_generated(generated_api().api_name##_set(to_generated_duty(duty_cycle_permille))); \
  }

RU_POSIX_DEFINE_PWM_OPS(pwm0, pwm_pwm_0)
RU_POSIX_DEFINE_PWM_OPS(pwm1, pwm_pwm_1)
RU_POSIX_DEFINE_PWM_OPS(pwm2, pwm_pwm_2)
RU_POSIX_DEFINE_PWM_OPS(pwm3, pwm_pwm_3)

#undef RU_POSIX_DEFINE_PWM_OPS

constexpr opaque_pwm make_opaque(const PwmId id) noexcept {
  switch (id) {
#define RU_POSIX_PWM_CASE(enum_name, wrapper_name)                                        \
    case PwmId::enum_name:                                                                 \
      return opaque_pwm{wrapper_name##_read, wrapper_name##_write};
    RU_POSIX_PWM_MAP(RU_POSIX_PWM_CASE)
#undef RU_POSIX_PWM_CASE
    default:
      return {};
  }
}
}  // namespace

Pwm::Pwm(const PwmId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
}

result Pwm::start() noexcept {
  return result::OK;
}

result Pwm::init() noexcept {
  return result::OK;
}

result Pwm::stop() noexcept {
  return result::OK;
}

result Pwm::enable() noexcept {
  return result::OK;
}

result Pwm::disable() noexcept {
  return m_opaque.write(0U);
}

result Pwm::set_frequency(const uint32_t frequency_hz) noexcept {
  (void)frequency_hz;
  return result::OK;
}

result Pwm::set_duty_cycle(const uint16_t duty_cycle_permille) noexcept {
  return m_opaque.write(duty_cycle_permille);
}

expected::expected<uint16_t, result> Pwm::get_duty_cycle() const noexcept {
  uint16_t duty_cycle_permille{0U};
  const auto status = m_opaque.read(duty_cycle_permille);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return duty_cycle_permille;
}
}  // namespace ru::driver
