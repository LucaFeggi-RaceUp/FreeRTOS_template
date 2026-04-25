#include "gpio.hpp"

#include "common.hpp"
#include "mapping.hpp"
#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
namespace {
result parse_gpio_value(const virtual_driver::Result& r_result, bool& r_value) noexcept {
  return parse_generated_bool(r_result, r_value) ? result::OK : result::RECOVERABLE_ERROR;
}

#define RU_POSIX_DEFINE_GPIO_OPS(wrapper_name, api_name)                                     \
  result wrapper_name##_read(bool& r_value) noexcept {                                       \
    return parse_gpio_value(generated_api().api_name##_read(), r_value);                     \
  }                                                                                          \
                                                                                             \
  result wrapper_name##_write(const bool value) noexcept {                                   \
    return result_from_generated(generated_api().api_name##_write(value));                   \
  }


RU_POSIX_DEFINE_GPIO_OPS(gpio0, gpio_gpio_0)
RU_POSIX_DEFINE_GPIO_OPS(gpio1, gpio_gpio_1)
RU_POSIX_DEFINE_GPIO_OPS(gpio2, gpio_gpio_2)
RU_POSIX_DEFINE_GPIO_OPS(gpio3, gpio_gpio_3)

#undef RU_POSIX_DEFINE_GPIO_OPS

constexpr opaque_gpio make_opaque(const GpioId id) noexcept {
  switch (id) {
#define RU_POSIX_GPIO_CASE(enum_name, wrapper_name, active_high, is_output)               \
    case GpioId::enum_name:                                                                \
      return opaque_gpio{wrapper_name##_read, wrapper_name##_write, active_high, is_output};
    RU_POSIX_GPIO_MAP(RU_POSIX_GPIO_CASE)
#undef RU_POSIX_GPIO_CASE
    default:
      return {};
  }
}
}  // namespace

Gpio::Gpio(const GpioId id) noexcept : m_id(id), m_opaque(make_opaque(id)) {
}

result Gpio::start() noexcept {
  return result::OK;
}

result Gpio::init() noexcept {
  return result::OK;
}

result Gpio::stop() noexcept {
  return result::OK;
}

GpioValue Gpio::active_value() const noexcept {
  return m_opaque.active_high() ? GpioValue::HIGH : GpioValue::LOW;
}

GpioPolarity Gpio::polarity() const noexcept {
  return m_opaque.active_high() ? GpioPolarity::ACTIVE_HIGH : GpioPolarity::ACTIVE_LOW;
}

expected::expected<bool, result> Gpio::is_active() const noexcept {
  const auto value = is_high();
  if (!value.has_value()) {
    return expected::unexpected(value.error());
  }

  return value.value() == m_opaque.active_high();
}

expected::expected<bool, result> Gpio::is_inactive() const noexcept {
  const auto value = is_active();
  if (!value.has_value()) {
    return expected::unexpected(value.error());
  }

  return !value.value();
}

expected::expected<bool, result> Gpio::is_high() const noexcept {
  bool value{false};
  const auto status = m_opaque.read(value);
  if (status != result::OK) {
    return expected::unexpected(status);
  }

  return value;
}

expected::expected<bool, result> Gpio::is_low() const noexcept {
  const auto value = is_high();
  if (!value.has_value()) {
    return expected::unexpected(value.error());
  }

  return !value.value();
}

result Gpio::set_active() noexcept {
  return m_opaque.write(m_opaque.active_high());
}

result Gpio::set_inactive() noexcept {
  return m_opaque.write(!m_opaque.active_high());
}

result Gpio::set_level(const bool active) noexcept {
  return m_opaque.write(active);
}

result Gpio::toggle() noexcept {
  const auto value = is_high();
  if (!value.has_value()) {
    return value.error();
  }

  return m_opaque.write(!value.value());
}
}  // namespace ru::driver
