#pragma once

#include <iostream>
#include <ostream>

namespace ru::driver {

inline struct {
  std::ostream& m_out;
} opaque_common_static{std::cout};

struct opaque_common {};

}  // namespace ru::driver
