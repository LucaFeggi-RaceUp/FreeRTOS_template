#include "opaque_serial.hpp"

#include <cerrno>
#include <chrono>

#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

using namespace ru::driver;

namespace ru::driver {
namespace {
result configure_fd(const int fd) noexcept {
  termios config{};
  if (tcgetattr(fd, &config) != 0) {
    return result::RECOVERABLE_ERROR;
  }

  cfmakeraw(&config);
  if (cfsetispeed(&config, B115200) != 0 || cfsetospeed(&config, B115200) != 0) {
    return result::RECOVERABLE_ERROR;
  }

  config.c_cflag |= CLOCAL | CREAD;
  config.c_cc[VMIN] = 0;
  config.c_cc[VTIME] = 0;

  return tcsetattr(fd, TCSANOW, &config) == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

int remaining_timeout_ms(const std::chrono::steady_clock::time_point deadline) noexcept {
  const auto now = std::chrono::steady_clock::now();
  if (now >= deadline) {
    return 0;
  }

  const auto remaining =
      std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
  return remaining > static_cast<int64_t>(INT32_MAX) ? INT32_MAX : static_cast<int>(remaining);
}
}  // namespace

result opaque_serial::init() noexcept {
  if (m_fd >= 0) {
    return result::OK;
  }

  m_fd = ::open(m_device_path.data(), O_RDWR | O_NOCTTY | O_CLOEXEC);
  if (m_fd < 0) {
    return result::RECOVERABLE_ERROR;
  }

  const auto status = configure_fd(m_fd);
  if (status != result::OK) {
    (void)::close(m_fd);
    m_fd = -1;
  }

  return status;
}

result opaque_serial::stop() noexcept {
  if (m_fd < 0) {
    return result::OK;
  }

  const auto close_status = ::close(m_fd);
  m_fd = -1;
  return close_status == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_serial::write(const uint8_t* const p_data, const size_t len,
                            const Timestamp timeout_uS) noexcept {
  if (len == 0U) {
    return result::OK;
  }

  const auto init_status = init();
  if (init_status != result::OK) {
    return init_status;
  }

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::microseconds(timeout_uS);

  size_t written{0U};
  while (written < len) {
    pollfd descriptor{m_fd, POLLOUT, 0};
    const auto poll_result =
        ::poll(&descriptor, 1, timeout_uS == 0U ? 0 : remaining_timeout_ms(deadline));
    if (poll_result == 0) {
      return result::RECOVERABLE_ERROR;
    }

    if (poll_result < 0) {
      if (errno == EINTR) {
        continue;
      }
      return result::RECOVERABLE_ERROR;
    }

    const auto chunk = ::write(m_fd, p_data + written, len - written);
    if (chunk < 0) {
      if (errno == EINTR) {
        continue;
      }
      return result::RECOVERABLE_ERROR;
    }

    written += static_cast<size_t>(chunk);
  }

  return tcdrain(m_fd) == 0 ? result::OK : result::RECOVERABLE_ERROR;
}

result opaque_serial::read(uint8_t* const p_data, const size_t len,
                           const Timestamp timeout_uS) noexcept {
  const auto init_status = init();
  if (init_status != result::OK) {
    return init_status;
  }

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::microseconds(timeout_uS);

  size_t read_bytes{0U};
  while (read_bytes < len) {
    pollfd descriptor{m_fd, POLLIN, 0};
    const auto poll_result =
        ::poll(&descriptor, 1, timeout_uS == 0U ? 0 : remaining_timeout_ms(deadline));
    if (poll_result == 0) {
      return result::RECOVERABLE_ERROR;
    }

    if (poll_result < 0) {
      if (errno == EINTR) {
        continue;
      }
      return result::RECOVERABLE_ERROR;
    }

    const auto chunk = ::read(m_fd, p_data + read_bytes, len - read_bytes);
    if (chunk < 0) {
      if (errno == EINTR) {
        continue;
      }
      return result::RECOVERABLE_ERROR;
    }

    if (chunk == 0) {
      return result::RECOVERABLE_ERROR;
    }

    read_bytes += static_cast<size_t>(chunk);
  }

  return result::OK;
}
}  // namespace ru::driver
