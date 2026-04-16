#pragma once

#include <cstdint>

#include "common.hpp"
#include "opaque_timer.hpp"
#include "timer_id.hpp"

namespace ru::driver {

class Timer {
 public:
  Timer(TimerId id) noexcept;
  static result start() noexcept;
  result init() noexcept;
  result stop() noexcept;

  Timestamp time_now() const noexcept;

  TimerId inline id() const noexcept { return m_id; }

 private:
  TimerId m_id;
  struct opaque_timer m_opaque;
};

}  // namespace ru::driver
