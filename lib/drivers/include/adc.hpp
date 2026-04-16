#pragma once

#include <cstdint>
#include <optional>

#include "adc_id.hpp"
#include "common.hpp"
#include "opaque_adc.hpp"

namespace ru::driver {

class Adc {
 public:
  Adc(AdcId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  expected::expected<uint16_t, result> read() noexcept;
  expected::expected<std::optional<uint16_t>, result> try_read() noexcept;

  AdcId inline id() const noexcept { return m_id; }

 private:
  AdcId m_id;
  struct opaque_adc m_opaque;
};

}  // namespace ru::driver
