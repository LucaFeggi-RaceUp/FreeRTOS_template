#pragma once

#include <cstdint>

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#include <expected>
namespace expected = std;
#else
#include <tl/expected.hpp>
namespace expected = tl;
#endif

using Timestamp = uint64_t;

namespace ru::driver {
enum class result : uint8_t { OK, RECOVERABLE_ERROR, UNRECOVERABLE_ERROR };
}  // namespace ru::driver

#include "opaque_common.hpp"

namespace ru::driver {

#define TRY_WHILE_RECOVERABLE(expr) \
  do {                           \
  } while ((expr) == result::RECOVERABLE_ERROR)

class Common {
 public:
  Common() noexcept;
  static result start() noexcept;

 private:
  struct opaque_common m_opaque;
};

}  // namespace ru::driver
