#include "clock.hpp"

#include <chrono>

using namespace ru::driver;

namespace ru::driver {
namespace {
std::chrono::steady_clock::time_point g_start_time = std::chrono::steady_clock::now();
bool g_started{false};

void ensure_started() noexcept {
  if (!g_started) {
    g_start_time = std::chrono::steady_clock::now();
    g_started = true;
  }
}
}  // namespace

result Clock::start() noexcept {
  g_start_time = std::chrono::steady_clock::now();
  g_started = true;
  return result::OK;
}

Timestamp Clock::now_us() noexcept {
  ensure_started();

  const auto elapsed = std::chrono::steady_clock::now() - g_start_time;
  return static_cast<Timestamp>(
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
}  // namespace ru::driver
