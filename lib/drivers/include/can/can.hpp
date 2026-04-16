#pragma once

#include <cstdint>
#include <optional>
#include "common.hpp"

namespace ru::driver {

struct CanMessage {
  uint32_t id : 19;
  uint32_t len : 4;
  union {
    uint8_t bytes[8];
    uint32_t words[2];
    uint64_t full_word;
  };
};

struct CanMessageTs {
  CanMessage message;
  std::optional<uint8_t> filter;
  Timestamp Ts;
};

}  // namespace ru::driver
