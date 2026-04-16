#include "timer.hpp"

#include <chrono>

using namespace ru::driver;

namespace ru::driver {
namespace {
std::chrono::steady_clock::time_point g_start_time = std::chrono::steady_clock::now();
}  // namespace

Timer::Timer(const TimerId id) noexcept : m_id(id) {
}

result Timer::start() noexcept {
  g_start_time = std::chrono::steady_clock::now();
  return result::OK;
}

result Timer::init() noexcept {
  return result::OK;
}

result Timer::stop() noexcept {
  return result::OK;
}

Timestamp Timer::time_now() const noexcept {
  const auto elapsed = std::chrono::steady_clock::now() - g_start_time;
  return static_cast<Timestamp>(
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
}  // namespace ru::driver
