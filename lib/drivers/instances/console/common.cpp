#include "common.hpp"

#include "utils/common.hpp"

using namespace ru::driver;


namespace ru::driver {
Common::Common() noexcept {
  LOG("Common driver object created");
}

result Common::start() noexcept {
  LOG("Common driver started");
  return result::OK;
}
}  // namespace ru::driver
