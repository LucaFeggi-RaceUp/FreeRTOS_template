#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <variant>

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

inline constexpr uint8_t k_invalid_can_fd_dlc = 0xFFU;

constexpr uint8_t can_fd_dlc_from_length(const uint8_t len) noexcept {
  switch (len) {
    case 0U:
    case 1U:
    case 2U:
    case 3U:
    case 4U:
    case 5U:
    case 6U:
    case 7U:
    case 8U:
      return len;
    case 12U:
      return 9U;
    case 16U:
      return 10U;
    case 20U:
      return 11U;
    case 24U:
      return 12U;
    case 32U:
      return 13U;
    case 48U:
      return 14U;
    case 64U:
      return 15U;
    default:
      return k_invalid_can_fd_dlc;
  }
}

constexpr uint8_t can_fd_length_from_dlc(const uint8_t dlc) noexcept {
  switch (dlc) {
    case 0U:
    case 1U:
    case 2U:
    case 3U:
    case 4U:
    case 5U:
    case 6U:
    case 7U:
    case 8U:
      return dlc;
    case 9U:
      return 12U;
    case 10U:
      return 16U;
    case 11U:
      return 20U;
    case 12U:
      return 24U;
    case 13U:
      return 32U;
    case 14U:
      return 48U;
    case 15U:
      return 64U;
    default:
      return 0U;
  }
}

struct CanFrameView {
  CanId id{};
  CanFrameFormat frame_format{CanFrameFormat::Classic};
  const uint8_t* data{nullptr};
  uint8_t len{0U};

  constexpr bool is_valid() const noexcept {
    return id.is_valid() && can_data_length_is_valid(frame_format, len) &&
           (len == 0U || data != nullptr);
  }
};

template <CanIdFormat IdFmt>
using can_header_word_t =
    std::conditional_t<IdFmt == CanIdFormat::Standard, uint16_t, uint64_t>;

template <typename HeaderWord, CanIdFormat IdFmt, std::size_t Capacity, bool IsFd>
class PackedCanMessage final {
 public:
  using self_type = PackedCanMessage<HeaderWord, IdFmt, Capacity, IsFd>;

  static_assert(std::is_unsigned<HeaderWord>::value, "HeaderWord must be unsigned");

  static constexpr CanIdFormat k_id_format = IdFmt;
  static constexpr std::size_t k_capacity = Capacity;
  static constexpr bool k_is_fd = IsFd;
  static constexpr uint8_t k_id_bits =
      IdFmt == CanIdFormat::Standard ? 11U : 29U;

  PackedCanMessage() noexcept = default;

  bool assign(const uint32_t raw_id,
              const CanFrameFormat frame_format,
              const uint8_t* const payload,
              const uint8_t len) noexcept {
    if (!can_id_value_is_valid(IdFmt, raw_id)) {
      return false;
    }
    if (len != 0U && payload == nullptr) {
      return false;
    }

    if constexpr (IsFd) {
      if (frame_format == CanFrameFormat::Classic) {
        return false;
      }
      const uint8_t dlc = can_fd_dlc_from_length(len);
      if (dlc == k_invalid_can_fd_dlc) {
        return false;
      }
      set_len_field(dlc);
      set_brs(frame_format == CanFrameFormat::FdBrs);
    } else {
      if (frame_format != CanFrameFormat::Classic || len > k_can_classic_max_payload) {
        return false;
      }
      set_len_field(len);
    }

    set_id(raw_id);
    if (len != 0U) {
      std::memcpy(payload_.data(), payload, len);
    }
    return true;
  }

  static expected::expected<self_type, result> make(const uint32_t raw_id,
                                                    const CanFrameFormat frame_format,
                                                    const uint8_t* const payload,
                                                    const uint8_t len) noexcept {
    self_type message{};
    if (!message.assign(raw_id, frame_format, payload, len)) {
      return expected::unexpected(result::UNRECOVERABLE_ERROR);
    }
    return message;
  }

  constexpr bool is_valid() const noexcept {
    return can_id_value_is_valid(IdFmt, raw_id()) &&
           can_data_length_is_valid(frame_format(), len());
  }

  constexpr CanId id() const noexcept {
    if constexpr (IdFmt == CanIdFormat::Standard) {
      return CanId::standard_unchecked(raw_id());
    } else {
      return CanId::extended_unchecked(raw_id());
    }
  }

  constexpr uint8_t len() const noexcept {
    return IsFd ? can_fd_length_from_dlc(raw_len_field()) : raw_len_field();
  }

  constexpr CanFrameFormat frame_format() const noexcept {
    if constexpr (IsFd) {
      return brs() ? CanFrameFormat::FdBrs : CanFrameFormat::Fd;
    } else {
      return CanFrameFormat::Classic;
    }
  }

  constexpr const std::array<uint8_t, Capacity>& payload() const noexcept {
    return payload_;
  }

  constexpr std::array<uint8_t, Capacity>& payload() noexcept { return payload_; }

  constexpr CanFrameView view() const noexcept {
    return CanFrameView{id(), frame_format(), payload_.data(), len()};
  }

 private:
  static constexpr uint8_t k_len_shift = k_id_bits;
  static constexpr HeaderWord k_id_mask =
      static_cast<HeaderWord>((static_cast<HeaderWord>(1U) << k_id_bits) - 1U);
  static constexpr HeaderWord k_len_mask =
      static_cast<HeaderWord>(static_cast<HeaderWord>(0x0FU) << k_len_shift);
  static constexpr uint8_t k_brs_shift = k_id_bits + 4U;
  static constexpr HeaderWord k_brs_mask =
      IsFd ? static_cast<HeaderWord>(static_cast<HeaderWord>(1U) << k_brs_shift)
           : static_cast<HeaderWord>(0U);

  constexpr uint32_t raw_id() const noexcept {
    return static_cast<uint32_t>(meta_ & k_id_mask);
  }

  constexpr uint8_t raw_len_field() const noexcept {
    return static_cast<uint8_t>((meta_ & k_len_mask) >> k_len_shift);
  }

  constexpr bool brs() const noexcept {
    return IsFd ? ((meta_ & k_brs_mask) != static_cast<HeaderWord>(0U)) : false;
  }

  void set_id(const uint32_t raw_id) noexcept {
    meta_ = static_cast<HeaderWord>(
        (meta_ & static_cast<HeaderWord>(~k_id_mask)) |
        static_cast<HeaderWord>(raw_id));
  }

  void set_len_field(const uint8_t value) noexcept {
    meta_ = static_cast<HeaderWord>(
        (meta_ & static_cast<HeaderWord>(~k_len_mask)) |
        (static_cast<HeaderWord>(value) << k_len_shift));
  }

  void set_brs(const bool on) noexcept {
    if constexpr (IsFd) {
      if (on) {
        meta_ = static_cast<HeaderWord>(meta_ | k_brs_mask);
      } else {
        meta_ =
            static_cast<HeaderWord>(meta_ & static_cast<HeaderWord>(~k_brs_mask));
      }
    }
  }

  HeaderWord meta_{0U};
  std::array<uint8_t, Capacity> payload_{};
};

template <CanIdFormat IdFmt>
using ClassicCanMessage =
    PackedCanMessage<can_header_word_t<IdFmt>, IdFmt, k_can_classic_max_payload, false>;

template <CanIdFormat IdFmt>
using FdCanMessage =
    PackedCanMessage<can_header_word_t<IdFmt>, IdFmt, k_can_fd_max_payload, true>;

using ClassicStdCanMessage = ClassicCanMessage<CanIdFormat::Standard>;
using ClassicExtCanMessage = ClassicCanMessage<CanIdFormat::Extended>;
using FdStdCanMessage = FdCanMessage<CanIdFormat::Standard>;
using FdExtCanMessage = FdCanMessage<CanIdFormat::Extended>;

inline constexpr CanFrameView make_can_frame_view(const CanFrameView frame) noexcept {
  return frame;
}

template <typename Message>
inline auto make_can_frame_view(const Message& message) noexcept
    -> decltype(message.view()) {
  return message.view();
}

using CanMessage =
    std::variant<ClassicStdCanMessage, ClassicExtCanMessage, FdStdCanMessage, FdExtCanMessage>;

inline CanFrameView make_can_frame_view(const CanMessage& message) noexcept {
  return std::visit([](const auto& frame) { return frame.view(); }, message);
}

namespace can_message {

inline expected::expected<ClassicStdCanMessage, result> classic_standard(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return ClassicStdCanMessage::make(id, CanFrameFormat::Classic, data, len);
}

inline expected::expected<ClassicExtCanMessage, result> classic_extended(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return ClassicExtCanMessage::make(id, CanFrameFormat::Classic, data, len);
}

inline expected::expected<FdStdCanMessage, result> fd_standard(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return FdStdCanMessage::make(id, CanFrameFormat::Fd, data, len);
}

inline expected::expected<FdExtCanMessage, result> fd_extended(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return FdExtCanMessage::make(id, CanFrameFormat::Fd, data, len);
}

inline expected::expected<FdStdCanMessage, result> fd_brs_standard(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return FdStdCanMessage::make(id, CanFrameFormat::FdBrs, data, len);
}

inline expected::expected<FdExtCanMessage, result> fd_brs_extended(
    const uint32_t id, const uint8_t* const data, const uint8_t len) noexcept {
  return FdExtCanMessage::make(id, CanFrameFormat::FdBrs, data, len);
}

}  // namespace can_message

struct CanMessageTs {
  CanMessage message;
  std::optional<uint8_t> filter;
  Timestamp Ts;
};

static_assert(sizeof(ClassicStdCanMessage) == 10U, "unexpected ClassicStdCanMessage size");
static_assert(sizeof(ClassicExtCanMessage) == 16U, "unexpected ClassicExtCanMessage size");
static_assert(sizeof(FdStdCanMessage) == 66U, "unexpected FdStdCanMessage size");
static_assert(sizeof(FdExtCanMessage) == 72U, "unexpected FdExtCanMessage size");

static_assert(std::is_trivially_copyable<ClassicStdCanMessage>::value,
              "ClassicStdCanMessage must remain trivial");
static_assert(std::is_trivially_copyable<ClassicExtCanMessage>::value,
              "ClassicExtCanMessage must remain trivial");
static_assert(std::is_trivially_copyable<FdStdCanMessage>::value,
              "FdStdCanMessage must remain trivial");
static_assert(std::is_trivially_copyable<FdExtCanMessage>::value,
              "FdExtCanMessage must remain trivial");

}  // namespace ru::driver
