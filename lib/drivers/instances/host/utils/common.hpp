#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string_view>
#include <type_traits>

#include "common.hpp"
#include "opaque_common.hpp"

namespace ru::driver {

#define LOG(expr)                                               \
  do {                                                          \
    std::ostringstream _oss;                                    \
    _oss << expr;                                               \
    log_with_timestamp(opaque_common_static.m_out, _oss.str()); \
  } while (0)

template <typename T,
          typename std::enable_if<std::is_enum<T>::value, int>::type = 0>
constexpr auto raw_value(const T value) noexcept ->
    typename std::underlying_type<T>::type {
  return static_cast<typename std::underlying_type<T>::type>(value);
}

template <typename T,
          typename std::enable_if<!std::is_enum<T>::value, int>::type = 0>
constexpr T raw_value(const T value) noexcept {
  return value;
}

template <typename T>
inline void log_with_timestamp(std::ostream& r_os, const T& r_text) {
  using namespace std::chrono;

  auto now = system_clock::now();
  auto ns = duration_cast<nanoseconds>(now.time_since_epoch()).count();

  long long sec = ns / 1'000'000'000LL;
  ns %= 1'000'000'000LL;

  long long milli = ns / 1'000'000LL;
  ns %= 1'000'000LL;

  long long micro = ns / 1'000LL;
  long long nano = ns % 1'000LL;

  r_os << '[' << sec << '.' << std::setw(3) << std::setfill('0') << milli << '.'
       << std::setw(3) << std::setfill('0') << micro << '.' << std::setw(3)
       << std::setfill('0') << nano << "] " << r_text << '\n';
}
}  // namespace ru::driver
