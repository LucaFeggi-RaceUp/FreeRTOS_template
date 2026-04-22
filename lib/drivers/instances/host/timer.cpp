#include "timer.hpp"

#include <array>
#include <chrono>

using namespace ru::driver;

namespace ru::driver {
namespace {
using SteadyClock = std::chrono::steady_clock;

struct TimerState {
  SteadyClock::time_point start{};
  bool running{false};
};

std::array<TimerState, static_cast<std::size_t>(TimerId::COUNT)> g_timers{};

TimerState* timer_state(const TimerId id) noexcept {
  const auto index = static_cast<std::size_t>(id);
  return index < g_timers.size() ? &g_timers[index] : nullptr;
}
}  // namespace

Timer::Timer(const TimerId id) noexcept : m_id(id) {
}

result Timer::start() noexcept {
  return result::OK;
}

result Timer::init() noexcept {
  auto* const p_state = timer_state(m_id);
  if (p_state == nullptr) {
    return result::RECOVERABLE_ERROR;
  }

  p_state->start = SteadyClock::now();
  p_state->running = true;
  return result::OK;
}

result Timer::stop() noexcept {
  auto* const p_state = timer_state(m_id);
  if (p_state == nullptr) {
    return result::RECOVERABLE_ERROR;
  }

  p_state->running = false;
  return result::OK;
}

expected::expected<TimerInstant, result> Timer::time_now() const noexcept {
  const auto* const p_state = timer_state(m_id);
  if (p_state == nullptr || !p_state->running) {
    return expected::unexpected(result::RECOVERABLE_ERROR);
  }

  const auto elapsed = SteadyClock::now() - p_state->start;
  return static_cast<TimerInstant>(
      std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
}  // namespace ru::driver
