#pragma once

#include <common.hpp>
#include "virtual_driver_test.h"

namespace ru::driver {
virtual_driver::VirtualDriverTest& generated_api() noexcept;
result result_from_generated(const virtual_driver::Result& r_result) noexcept;
const char* trim_generated_value(const virtual_driver::Result& r_result) noexcept;
bool parse_generated_bool(const virtual_driver::Result& r_result, bool& r_value) noexcept;
bool parse_generated_double(const virtual_driver::Result& r_result, double& r_value) noexcept;
}  // namespace ru::driver
