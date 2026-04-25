#include "opaque_nv_memory.hpp"

#include <array>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace ru::driver;

namespace ru::driver {
namespace {
constexpr uint8_t k_erased_value{0xFFU};

result write_fill(const int fd, const uint32_t addr, const size_t len) noexcept {
  std::array<uint8_t, 256> fill{};
  fill.fill(k_erased_value);

  size_t remaining{len};
  uint32_t offset{addr};
  while (remaining > 0U) {
    const auto chunk = remaining < fill.size() ? remaining : fill.size();
    if (::pwrite(fd, fill.data(), chunk, static_cast<off_t>(offset)) !=
        static_cast<ssize_t>(chunk)) {
      return result::RECOVERABLE_ERROR;
    }

    remaining -= chunk;
    offset += static_cast<uint32_t>(chunk);
  }

  return result::OK;
}
}  // namespace

result opaque_nv_memory::ensure_ready() noexcept {
  if (m_path.empty() || m_capacity == 0U) {
    return result::UNRECOVERABLE_ERROR;
  }

  if (m_fd >= 0) {
    return result::OK;
  }

  m_fd = ::open(m_path.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, 0644);
  if (m_fd < 0) {
    return result::RECOVERABLE_ERROR;
  }

  struct stat info {};
  if (::fstat(m_fd, &info) != 0) {
    (void)stop();
    return result::RECOVERABLE_ERROR;
  }

  const auto current_size = static_cast<uint64_t>(info.st_size);
  const auto required_size =
      static_cast<uint64_t>(m_offset) + static_cast<uint64_t>(m_capacity);
  if (current_size >= required_size) {
    return result::OK;
  }

  if (::ftruncate(m_fd, static_cast<off_t>(required_size)) != 0) {
    (void)stop();
    return result::RECOVERABLE_ERROR;
  }

  const auto fill_status =
      write_fill(m_fd, static_cast<uint32_t>(current_size),
                 static_cast<size_t>(required_size - current_size));
  if (fill_status != result::OK) {
    (void)stop();
    return fill_status;
  }

  return ::fsync(m_fd) == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_nv_memory::init() noexcept {
  return ensure_ready();
}

result opaque_nv_memory::stop() noexcept {
  if (m_fd < 0) {
    return result::OK;
  }

  const auto close_status = ::close(m_fd);
  m_fd = -1;
  return close_status == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_nv_memory::clear() noexcept {
  const auto init_status = ensure_ready();
  if (init_status != result::OK) {
    return init_status;
  }

  const auto erase_status = write_fill(m_fd, m_offset, m_capacity);
  if (erase_status != result::OK) {
    return erase_status;
  }

  return ::fsync(m_fd) == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_nv_memory::read(const uint32_t address, uint8_t* const p_data,
                              const size_t len) noexcept {
  if ((len != 0U && p_data == nullptr) ||
      (static_cast<uint64_t>(address) + static_cast<uint64_t>(len) >
       static_cast<uint64_t>(m_capacity))) {
    return result::RECOVERABLE_ERROR;
  }

  const auto init_status = ensure_ready();
  if (init_status != result::OK) {
    return init_status;
  }

  if (len == 0U) {
    return result::OK;
  }

  return ::pread(m_fd, p_data, len,
                 static_cast<off_t>(static_cast<uint64_t>(m_offset) + address)) ==
                 static_cast<ssize_t>(len)
             ? result::OK
             : result::RECOVERABLE_ERROR;
}

result opaque_nv_memory::write(const uint32_t address, const uint8_t* const p_data,
                               const size_t len) noexcept {
  if ((len != 0U && p_data == nullptr) ||
      (static_cast<uint64_t>(address) + static_cast<uint64_t>(len) >
       static_cast<uint64_t>(m_capacity))) {
    return result::RECOVERABLE_ERROR;
  }

  const auto init_status = ensure_ready();
  if (init_status != result::OK) {
    return init_status;
  }

  if (len == 0U) {
    return result::OK;
  }

  if (::pwrite(m_fd, p_data, len,
               static_cast<off_t>(static_cast<uint64_t>(m_offset) + address)) !=
      static_cast<ssize_t>(len)) {
    return result::RECOVERABLE_ERROR;
  }

  return ::fsync(m_fd) == 0 ? result::OK : result::RECOVERABLE_ERROR;
}
}  // namespace ru::driver
