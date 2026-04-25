#include "common.hpp"

#include <cstdlib>
#include <cstring>

#include "utils/common.hpp"

using namespace ru::driver;

namespace ru::driver {
virtual_driver::VirtualDriverTest& generated_api() noexcept {
  static virtual_driver::VirtualDriverTest api{RU_POSIX_GENERATED_SOCKET};
  return api;
}

result result_from_generated(const virtual_driver::Result& r_result) noexcept {
  return r_result.ok ? result::OK : result::RECOVERABLE_ERROR;
}

const char* trim_generated_value(const virtual_driver::Result& r_result) noexcept {
  const auto* p_value = r_result.value.c_str();
  while (*p_value == ' ' || *p_value == '\t' || *p_value == '\r' || *p_value == '\n') {
    ++p_value;
  }

  return p_value;
}

bool parse_generated_bool(const virtual_driver::Result& r_result, bool& r_value) noexcept {
  if (!r_result.ok) {
    return false;
  }

  const auto* p_value = trim_generated_value(r_result);
  if (std::strcmp(p_value, "1") == 0 || std::strcmp(p_value, "true") == 0 ||
      std::strcmp(p_value, "TRUE") == 0) {
    r_value = true;
    return true;
  }

  if (std::strcmp(p_value, "0") == 0 || std::strcmp(p_value, "false") == 0 ||
      std::strcmp(p_value, "FALSE") == 0) {
    r_value = false;
    return true;
  }

  return false;
}

bool parse_generated_double(const virtual_driver::Result& r_result, double& r_value) noexcept {
  if (!r_result.ok || r_result.value.empty()) {
    return false;
  }

  char* p_end{nullptr};
  const auto* p_value = trim_generated_value(r_result);
  const auto value = std::strtod(p_value, &p_end);
  while (p_end != nullptr && (*p_end == ' ' || *p_end == '\t' || *p_end == '\r' || *p_end == '\n')) {
    ++p_end;
  }
  if (p_end == p_value || (p_end != nullptr && *p_end != '\0')) {
    return false;
  }

  r_value = value;
  return true;
}
}  // namespace ru::driver
