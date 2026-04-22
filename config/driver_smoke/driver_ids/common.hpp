#pragma once
#include <array>
#include <cstdint>
#include <string_view>

#define ENUM_VALUE(name) name,
#define ENUM_STRING(name) #name,

#define DECLARE_ID_ENUM(Name, LIST)                                       \
                                                                          \
  enum class Name##Id : uint8_t{LIST(ENUM_VALUE) COUNT};                  \
                                                                          \
  inline constexpr std::array<std::string_view,                           \
                              static_cast<size_t>(Name##Id::COUNT)>       \
      Name##_strings = {LIST(ENUM_STRING)};                               \
                                                                          \
  inline constexpr std::string_view toText(Name##Id id) noexcept {        \
    const auto idx = static_cast<size_t>(id);                             \
    return idx < Name##_strings.size() ? Name##_strings[idx] : "UNKNOWN"; \
  }
