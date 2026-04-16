#pragma once

#include <cstddef>
#include <cstdint>

#include "common.hpp"

namespace ru::driver {
class Serial;

struct opaque_serial {
 public:
  constexpr opaque_serial() noexcept = default;

 private:
  friend class Serial;

  result init() const noexcept;
  result stop() const noexcept;
  result write(const uint8_t* p_data, size_t len, Timestamp timeout_uS) const noexcept;
  result read(uint8_t* p_data, size_t len, Timestamp timeout_uS) const noexcept;
};
}  // namespace ru::driver
