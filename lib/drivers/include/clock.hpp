#pragma once

#include "common.hpp"

namespace ru::driver {

class Clock {
 public:
  static result start() noexcept;

  // Monotonic time in microseconds. Resolution is backend-dependent.
  static Timestamp now_us() noexcept;
};

}  // namespace ru::driver
