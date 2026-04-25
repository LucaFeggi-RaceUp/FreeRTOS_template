#include "opaque_timer.hpp"

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

result opaque_timer::start() noexcept {
  g_start_time = std::chrono::steady_clock::now();
  g_started = true;
  return result::OK;
}

result opaque_timer::init() const noexcept {
  ensure_started();
  return result::OK;
}

result opaque_timer::stop() const noexcept {
  return result::OK;
}

Timestamp opaque_timer::time_now() const noexcept {
  ensure_started();
  const auto elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now() - g_start_time)
          .count();
  return elapsed < 0 ? 0U : static_cast<Timestamp>(elapsed);
}
}  // namespace ru::driver
