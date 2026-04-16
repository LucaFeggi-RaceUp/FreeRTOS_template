#pragma once

#include "common.hpp"

namespace ru::driver {
class Common;

struct opaque_common {
 private:
  friend class Common;

  result start() const noexcept;
};
}  // namespace ru::driver
