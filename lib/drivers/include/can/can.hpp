#pragma once

#include <cstring>
#include <cstdint>
#include <optional>

#include "common.hpp"

namespace ru::driver {

inline constexpr uint32_t k_can_standard_id_mask = 0x7FFU;
inline constexpr uint32_t k_can_extended_id_mask = 0x1FFFFFFFU;
inline constexpr uint8_t k_can_classic_max_payload = 8U;
inline constexpr uint8_t k_can_fd_max_payload = 64U;

enum class CanIdFormat : uint8_t { Standard, Extended };

enum class CanFrameFormat : uint8_t { Classic, Fd, FdBrs };

constexpr uint32_t can_id_mask(const CanIdFormat format) noexcept {
  return format == CanIdFormat::Extended ? k_can_extended_id_mask
                                         : k_can_standard_id_mask;
}

constexpr bool can_id_value_is_valid(const CanIdFormat format,
                                     const uint32_t value) noexcept {
  return value <= can_id_mask(format);
}

constexpr bool can_data_length_is_valid(const CanFrameFormat frame_format,
                                        const uint8_t len) noexcept {
  if (frame_format == CanFrameFormat::Classic) {
    return len <= k_can_classic_max_payload;
  }

  return len <= 8U || len == 12U || len == 16U || len == 20U || len == 24U ||
         len == 32U || len == 48U || len == 64U;
}

constexpr uint8_t can_payload_capacity(const CanFrameFormat frame_format) noexcept {
  return frame_format == CanFrameFormat::Classic ? k_can_classic_max_payload
                                                 : k_can_fd_max_payload;
}

constexpr bool can_frame_format_is_allowed(const CanFrameFormat controller_format,
                                           const CanFrameFormat message_format) noexcept {
  if (message_format == CanFrameFormat::Classic) {
    return true;
  }
  if (message_format == CanFrameFormat::Fd) {
    return controller_format == CanFrameFormat::Fd ||
           controller_format == CanFrameFormat::FdBrs;
  }

  return controller_format == CanFrameFormat::FdBrs;
}

struct CanId {
  uint32_t value{0U};
  CanIdFormat format{CanIdFormat::Standard};

  static expected::expected<CanId, result> standard(uint32_t value) noexcept {
    if (!can_id_value_is_valid(CanIdFormat::Standard, value)) {
      return expected::unexpected(result::UNRECOVERABLE_ERROR);
    }
    return CanId{value, CanIdFormat::Standard};
  }

  static expected::expected<CanId, result> extended(uint32_t value) noexcept {
    if (!can_id_value_is_valid(CanIdFormat::Extended, value)) {
      return expected::unexpected(result::UNRECOVERABLE_ERROR);
    }
    return CanId{value, CanIdFormat::Extended};
  }

  static constexpr CanId standard_unchecked(uint32_t value) noexcept {
    return CanId{value & k_can_standard_id_mask, CanIdFormat::Standard};
  }

  static constexpr CanId extended_unchecked(uint32_t value) noexcept {
    return CanId{value & k_can_extended_id_mask, CanIdFormat::Extended};
  }

  constexpr bool is_valid() const noexcept {
    return can_id_value_is_valid(format, value);
  }
};

struct CanControllerConfig {
  CanFrameFormat max_frame_format{CanFrameFormat::FdBrs};
  uint8_t standard_filter_count{28U};
  uint8_t extended_filter_count{28U};
};

struct CanMessage {
  CanId id{};
  uint8_t len{0U};
  CanFrameFormat frame_format{CanFrameFormat::Classic};

  union {
    uint8_t bytes[k_can_fd_max_payload];
    uint32_t words[k_can_fd_max_payload / sizeof(uint32_t)];
    uint64_t full_word;
    uint64_t full_words[k_can_fd_max_payload / sizeof(uint64_t)];
  };

  CanMessage() noexcept
      : id{}, len{0U}, frame_format{CanFrameFormat::Classic}, bytes{} {}

  bool is_valid() const noexcept {
    return id.is_valid() && can_data_length_is_valid(frame_format, len);
  }

  static expected::expected<CanMessage, result> make(CanId id,
                                                     CanFrameFormat frame_format,
                                                     const uint8_t* data,
                                                     uint8_t len) noexcept {
    if (!id.is_valid() || !can_data_length_is_valid(frame_format, len) ||
        (len != 0U && data == nullptr)) {
      return expected::unexpected(result::UNRECOVERABLE_ERROR);
    }

    CanMessage message{};
    message.id = id;
    message.len = len;
    message.frame_format = frame_format;
    if (len != 0U) {
      std::memcpy(message.bytes, data, len);
    }
    return message;
  }

  static expected::expected<CanMessage, result> classic_standard(uint32_t id,
                                                                 const uint8_t* data,
                                                                 uint8_t len) noexcept {
    const auto can_id = CanId::standard(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::Classic, data, len);
  }

  static expected::expected<CanMessage, result> classic_extended(uint32_t id,
                                                                 const uint8_t* data,
                                                                 uint8_t len) noexcept {
    const auto can_id = CanId::extended(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::Classic, data, len);
  }

  static expected::expected<CanMessage, result> fd_standard(uint32_t id,
                                                           const uint8_t* data,
                                                           uint8_t len) noexcept {
    const auto can_id = CanId::standard(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::Fd, data, len);
  }

  static expected::expected<CanMessage, result> fd_extended(uint32_t id,
                                                           const uint8_t* data,
                                                           uint8_t len) noexcept {
    const auto can_id = CanId::extended(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::Fd, data, len);
  }

  static expected::expected<CanMessage, result> fd_brs_standard(uint32_t id,
                                                               const uint8_t* data,
                                                               uint8_t len) noexcept {
    const auto can_id = CanId::standard(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::FdBrs, data, len);
  }

  static expected::expected<CanMessage, result> fd_brs_extended(uint32_t id,
                                                               const uint8_t* data,
                                                               uint8_t len) noexcept {
    const auto can_id = CanId::extended(id);
    if (!can_id.has_value()) {
      return expected::unexpected(can_id.error());
    }
    return make(can_id.value(), CanFrameFormat::FdBrs, data, len);
  }
};

struct CanMessageTs {
  CanMessage message;
  std::optional<uint8_t> filter;
  Timestamp Ts;
};

}  // namespace ru::driver
