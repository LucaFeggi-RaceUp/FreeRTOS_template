#include "smoke_test.hpp"

#include <cstdio>
#include <cstring>

namespace smoke {
namespace {
constexpr uint32_t k_nv_memory_magic = 0x52545344UL;
constexpr uint16_t k_nv_memory_version = 1U;

struct NvMemoryRecord {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t boot_counter;
  uint32_t checksum;
};

uint32_t checksum_record(const NvMemoryRecord& record) noexcept {
  NvMemoryRecord copy = record;
  copy.checksum = 0U;

  const auto* bytes = reinterpret_cast<const uint8_t*>(&copy);
  uint32_t checksum = 2166136261UL;
  for (std::size_t index = 0U; index < sizeof(copy); ++index) {
    checksum ^= bytes[index];
    checksum *= 16777619UL;
  }
  return checksum;
}

bool record_is_valid(const NvMemoryRecord& record) noexcept {
  return record.magic == k_nv_memory_magic &&
         record.version == k_nv_memory_version &&
         record.checksum == checksum_record(record);
}

ru::driver::result read_record(ru::driver::NvMemory& nv_memory,
                               NvMemoryRecord& record) noexcept {
  return nv_memory.read(0U, reinterpret_cast<uint8_t*>(&record), sizeof(record));
}

ru::driver::result write_record(ru::driver::NvMemory& nv_memory,
                                const NvMemoryRecord& record) noexcept {
  return nv_memory.write(0U, reinterpret_cast<const uint8_t*>(&record),
                         sizeof(record));
}
}  // namespace

void run_nv_memory_smoke(SmokeContext& context) noexcept {
  mark_result(context, ru::driver::NvMemory::start(), k_error_nv_memory);
  const auto init_status = context.nv_memory.init();
  mark_result(context, init_status, k_error_nv_memory);
  if (init_status != ru::driver::result::OK ||
      context.nv_memory.capacity() < sizeof(NvMemoryRecord)) {
    set_error(context, k_error_nv_memory);
    return;
  }

  NvMemoryRecord previous{};
  const auto read_status = read_record(context.nv_memory, previous);
  mark_result(context, read_status, k_error_nv_memory);

  const bool previous_valid =
      read_status == ru::driver::result::OK && record_is_valid(previous);
  const uint32_t previous_counter = previous_valid ? previous.boot_counter : 0U;

  NvMemoryRecord next{};
  next.magic = k_nv_memory_magic;
  next.version = k_nv_memory_version;
  next.reserved = 0U;
  next.boot_counter = previous_counter + 1U;
  next.checksum = checksum_record(next);

  mark_result(context, write_record(context.nv_memory, next), k_error_nv_memory);

  NvMemoryRecord readback{};
  const auto readback_status = read_record(context.nv_memory, readback);
  mark_result(context, readback_status, k_error_nv_memory);
  const bool readback_ok =
      readback_status == ru::driver::result::OK && record_is_valid(readback) &&
      readback.boot_counter == next.boot_counter;

  if (!readback_ok) {
    set_error(context, k_error_nv_memory);
  }

  context.nv_memory_summary.previous_boot_counter = previous_counter;
  context.nv_memory_summary.boot_counter = next.boot_counter;
  context.nv_memory_summary.checksum = next.checksum;
  context.nv_memory_summary.valid_previous = previous_valid ? 1U : 0U;
  context.nv_memory_summary.readback_ok = readback_ok ? 1U : 0U;

  char line[128]{};
  std::snprintf(
      line, sizeof(line),
      "nv_memory previous_valid=%u previous=%lu boot=%lu readback=%u\r\n",
      static_cast<unsigned>(context.nv_memory_summary.valid_previous),
      static_cast<unsigned long>(previous_counter),
      static_cast<unsigned long>(next.boot_counter),
      static_cast<unsigned>(context.nv_memory_summary.readback_ok));
  usb_line(context, line);
}

}  // namespace smoke
