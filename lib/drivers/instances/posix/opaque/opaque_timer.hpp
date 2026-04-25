#pragma once

#include "common.hpp"

namespace ru::driver {
class Timer;

struct opaque_timer {
 private:
  friend class Timer;

  static result start() noexcept;
  result init() const noexcept;
  result stop() const noexcept;
  Timestamp time_now() const noexcept;
};
}  // namespace ru::driver
