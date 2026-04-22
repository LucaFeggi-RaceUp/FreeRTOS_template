#include "smoke_test.hpp"

#include <cstdio>
#include <cstring>

namespace smoke {
namespace {
constexpr uint32_t k_eeprom_magic = 0x52545344UL;
constexpr uint16_t k_eeprom_version = 1U;

struct EepromRecord {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t boot_counter;
  uint32_t checksum;
};

uint32_t checksum_record(const EepromRecord& record) noexcept {
  EepromRecord copy = record;
  copy.checksum = 0U;

  const auto* bytes = reinterpret_cast<const uint8_t*>(&copy);
  uint32_t checksum = 2166136261UL;
  for (std::size_t index = 0U; index < sizeof(copy); ++index) {
    checksum ^= bytes[index];
    checksum *= 16777619UL;
  }
  return checksum;
}

bool record_is_valid(const EepromRecord& record) noexcept {
  return record.magic == k_eeprom_magic && record.version == k_eeprom_version &&
         record.checksum == checksum_record(record);
}

ru::driver::result read_record(ru::driver::Eeprom& eeprom,
                               EepromRecord& record) noexcept {
  return eeprom.read(0U, reinterpret_cast<uint8_t*>(&record), sizeof(record));
}

ru::driver::result write_record(ru::driver::Eeprom& eeprom,
                                const EepromRecord& record) noexcept {
  return eeprom.write(0U, reinterpret_cast<const uint8_t*>(&record), sizeof(record));
}
}  // namespace

void run_eeprom_smoke(SmokeContext& context) noexcept {
  mark_result(context, ru::driver::Eeprom::start(), k_error_eeprom);
  const auto init_status = context.eeprom.init();
  mark_result(context, init_status, k_error_eeprom);
  if (init_status != ru::driver::result::OK ||
      context.eeprom.capacity() < sizeof(EepromRecord)) {
    set_error(context, k_error_eeprom);
    return;
  }

  EepromRecord previous{};
  const auto read_status = read_record(context.eeprom, previous);
  mark_result(context, read_status, k_error_eeprom);

  const bool previous_valid =
      read_status == ru::driver::result::OK && record_is_valid(previous);
  const uint32_t previous_counter = previous_valid ? previous.boot_counter : 0U;

  EepromRecord next{};
  next.magic = k_eeprom_magic;
  next.version = k_eeprom_version;
  next.reserved = 0U;
  next.boot_counter = previous_counter + 1U;
  next.checksum = checksum_record(next);

  mark_result(context, write_record(context.eeprom, next), k_error_eeprom);

  EepromRecord readback{};
  const auto readback_status = read_record(context.eeprom, readback);
  mark_result(context, readback_status, k_error_eeprom);
  const bool readback_ok =
      readback_status == ru::driver::result::OK && record_is_valid(readback) &&
      readback.boot_counter == next.boot_counter;

  if (!readback_ok) {
    set_error(context, k_error_eeprom);
  }

  context.eeprom_summary.previous_boot_counter = previous_counter;
  context.eeprom_summary.boot_counter = next.boot_counter;
  context.eeprom_summary.checksum = next.checksum;
  context.eeprom_summary.valid_previous = previous_valid ? 1U : 0U;
  context.eeprom_summary.readback_ok = readback_ok ? 1U : 0U;

  char line[128]{};
  std::snprintf(line, sizeof(line),
                "eeprom previous_valid=%u previous=%lu boot=%lu readback=%u\r\n",
                static_cast<unsigned>(context.eeprom_summary.valid_previous),
                static_cast<unsigned long>(previous_counter),
                static_cast<unsigned long>(next.boot_counter),
                static_cast<unsigned>(context.eeprom_summary.readback_ok));
  usb_line(context, line);
}

}  // namespace smoke
