#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "opaque_nv_memory_flash_region.hpp"
#include "stm32h5xx_hal.h"

using namespace ru::driver;

extern "C" uint8_t __persist_flash_start__[];
extern "C" uint8_t __persist_flash_end__[];

namespace ru::driver {
namespace {
constexpr uint8_t k_erased_value{0xFFU};
constexpr size_t k_quadword_size{16U};

alignas(k_quadword_size) std::array<uint8_t, FLASH_SECTOR_SIZE> g_shadow_storage{};

uintptr_t storage_begin() noexcept {
  return reinterpret_cast<uintptr_t>(&__persist_flash_start__);
}

uintptr_t storage_end() noexcept {
  return reinterpret_cast<uintptr_t>(&__persist_flash_end__);
}

size_t storage_size() noexcept {
  return static_cast<size_t>(storage_end() - storage_begin());
}

bool in_bounds(const uint32_t addr, const size_t len) noexcept {
  return static_cast<uint64_t>(addr) + static_cast<uint64_t>(len) <=
         static_cast<uint64_t>(storage_size());
}

result validate_layout() noexcept {
  return storage_size() == g_shadow_storage.size() &&
                 (storage_size() % k_quadword_size) == 0U
             ? result::OK
             : result::UNRECOVERABLE_ERROR;
}

const uint8_t* storage_ptr(const uint32_t addr) noexcept {
  return reinterpret_cast<const uint8_t*>(storage_begin() +
                                          static_cast<uintptr_t>(addr));
}

uint32_t absolute_address(const size_t offset) noexcept {
  return static_cast<uint32_t>(storage_begin() + offset);
}

uint32_t sector_index(const uint32_t absolute_addr) noexcept {
  if (absolute_addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
    return (absolute_addr - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }

  return (absolute_addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_SECTOR_SIZE;
}

uint32_t bank_for_address(const uint32_t absolute_addr) noexcept {
  const auto bank_swapped =
      READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_SWAP_BANK) != 0U;
  const auto in_bank1_space = absolute_addr < (FLASH_BASE + FLASH_BANK_SIZE);

  if (!bank_swapped) {
    return in_bank1_space ? FLASH_BANK_1 : FLASH_BANK_2;
  }

  return in_bank1_space ? FLASH_BANK_2 : FLASH_BANK_1;
}

bool is_erased_range(const uint8_t* const data, const size_t len) noexcept {
  return std::all_of(data, data + len,
                     [](const uint8_t value) { return value == k_erased_value; });
}

result load_shadow() noexcept {
  const auto layout_status = validate_layout();
  if (layout_status != result::OK) {
    return layout_status;
  }

  auto* const storage =
      reinterpret_cast<volatile const uint8_t*>(storage_begin());
  for (size_t index = 0U; index < storage_size(); ++index) {
    g_shadow_storage[index] = storage[index];
  }

  return result::OK;
}

result erase_storage_sector() noexcept {
  FLASH_EraseInitTypeDef erase_config{};
  uint32_t sector_error = 0U;
  const auto base_addr = absolute_address(0U);

  erase_config.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase_config.Banks = bank_for_address(base_addr);
  erase_config.Sector = sector_index(base_addr);
  erase_config.NbSectors = 1U;

  return HAL_FLASHEx_Erase(&erase_config, &sector_error) == HAL_OK
             ? result::OK
             : result::RECOVERABLE_ERROR;
}

result program_shadow() noexcept {
  const auto total_size = storage_size();
  for (size_t offset = 0U; offset < total_size; offset += k_quadword_size) {
    auto* const quadword = g_shadow_storage.data() + offset;
    if (is_erased_range(quadword, k_quadword_size)) {
      continue;
    }

    if (HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_QUADWORD, absolute_address(offset),
            reinterpret_cast<uint32_t>(quadword)) != HAL_OK) {
      return result::RECOVERABLE_ERROR;
    }
  }

  return result::OK;
}

template <typename Operation>
result with_unlocked_flash(const Operation& operation) noexcept {
  if (HAL_FLASH_Unlock() != HAL_OK) {
    return result::RECOVERABLE_ERROR;
  }

  const auto operation_status = operation();
  const auto lock_status = HAL_FLASH_Lock() == HAL_OK ? result::OK
                                                      : result::RECOVERABLE_ERROR;

  return operation_status != result::OK ? operation_status : lock_status;
}

result commit_shadow() noexcept {
  return with_unlocked_flash([]() noexcept {
    const auto erase_status = erase_storage_sector();
    if (erase_status != result::OK) {
      return erase_status;
    }

    return program_shadow();
  });
}
}  // namespace

result opaque_nv_memory_flash_region::init() noexcept {
  return validate_layout();
}

result opaque_nv_memory_flash_region::stop() noexcept {
  return result::OK;
}

uint32_t opaque_nv_memory_flash_region::capacity() const noexcept {
  return static_cast<uint32_t>(storage_size());
}

result opaque_nv_memory_flash_region::read(const uint32_t addr,
                                           uint8_t* const p_data,
                                           const size_t len) const noexcept {
  if (!in_bounds(addr, len) || (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  const auto layout_status = validate_layout();
  if (layout_status != result::OK || len == 0U) {
    return layout_status;
  }

  std::memcpy(p_data, storage_ptr(addr), len);
  return result::OK;
}

result opaque_nv_memory_flash_region::write(const uint32_t addr,
                                            const uint8_t* const p_data,
                                            const size_t len) noexcept {
  if (!in_bounds(addr, len) || (len != 0U && p_data == nullptr)) {
    return result::RECOVERABLE_ERROR;
  }

  const auto layout_status = validate_layout();
  if (layout_status != result::OK || len == 0U) {
    return layout_status;
  }

  if (std::memcmp(storage_ptr(addr), p_data, len) == 0) {
    return result::OK;
  }

  const auto shadow_status = load_shadow();
  if (shadow_status != result::OK) {
    return shadow_status;
  }

  std::memcpy(g_shadow_storage.data() + addr, p_data, len);
  return commit_shadow();
}

result opaque_nv_memory_flash_region::erase(const uint32_t addr,
                                            const size_t len) noexcept {
  if (!in_bounds(addr, len)) {
    return result::RECOVERABLE_ERROR;
  }

  const auto layout_status = validate_layout();
  if (layout_status != result::OK || len == 0U) {
    return layout_status;
  }

  if (is_erased_range(storage_ptr(addr), len)) {
    return result::OK;
  }

  const auto shadow_status = load_shadow();
  if (shadow_status != result::OK) {
    return shadow_status;
  }

  std::memset(g_shadow_storage.data() + addr, k_erased_value, len);
  return commit_shadow();
}
}  // namespace ru::driver
