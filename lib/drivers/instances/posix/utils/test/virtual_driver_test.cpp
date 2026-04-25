#include "virtual_driver_test.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>

#include <cerrno>
#include <cstring>
#include <sstream>

namespace virtual_driver {

static Result parse_response(const std::string& line) {
  Result r;
  if (line.rfind("OK", 0) == 0) {
    r.ok = true;
    r.value = line.substr(2);
    return r;
  }
  if (line.rfind("ERR", 0) == 0) {
    r.ok = false;
    r.error = line.substr(3);
    return r;
  }
  r.ok = false;
  r.error = "invalid_response";
  return r;
}

VirtualDriverTest::VirtualDriverTest(std::string socket_path)
    : socket_path_(std::move(socket_path)) {}

Event::Event(int fd) : fd_(fd) {}

Event::Event(Event&& other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

Event& Event::operator=(Event&& other) noexcept {
  if (this != &other) {
    if (fd_ >= 0) {
      ::close(fd_);
    }
    fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

Event::~Event() {
  if (fd_ >= 0) {
    ::close(fd_);
  }
}

bool Event::valid() const { return fd_ >= 0; }
int Event::fd() const { return fd_; }

uint64_t Event::wait() {
  if (fd_ < 0) {
    return 0;
  }
  struct pollfd pfd {
    fd_,
    POLLIN,
    0
  };
  while (true) {
    int rc = ::poll(&pfd, 1, -1);
    if (rc < 0 && errno == EINTR) {
      continue;
    }
    break;
  }
  uint64_t value = 0;
  ssize_t n = ::read(fd_, &value, sizeof(value));
  if (n != static_cast<ssize_t>(sizeof(value))) {
    return 0;
  }
  return value;
}

bool Event::try_wait(uint64_t& out) {
  if (fd_ < 0) {
    return false;
  }
  uint64_t value = 0;
  ssize_t n = ::read(fd_, &value, sizeof(value));
  if (n == sizeof(value)) {
    out = value;
    return true;
  }
  return false;
}

Result VirtualDriverTest::send_cmd(const std::string& cmd) {
  int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    return Result{false, "", "socket_failed"};
  }
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path_.c_str());
  if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(fd);
    return Result{false, "", "connect_failed"};
  }
  std::string line = cmd + "\n";
  if (::send(fd, line.c_str(), line.size(), 0) < 0) {
    ::close(fd);
    return Result{false, "", "send_failed"};
  }
  std::string buffer;
  char tmp[256];
  while (true) {
    ssize_t n = ::recv(fd, tmp, sizeof(tmp), 0);
    if (n <= 0) {
      ::close(fd);
      return Result{false, "", "recv_failed"};
    }
    buffer.append(tmp, tmp + n);
    auto pos = buffer.find('\n');
    if (pos != std::string::npos) {
      std::string resp = buffer.substr(0, pos);
      ::close(fd);
      return parse_response(resp);
    }
  }
}

EventResult VirtualDriverTest::send_cmd_with_fd(const std::string& cmd) {
  int sock = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0) {
    return EventResult{false, Event(), "socket_failed"};
  }
  sockaddr_un addr{};
  addr.sun_family = AF_UNIX;
  std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socket_path_.c_str());
  if (::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    ::close(sock);
    return EventResult{false, Event(), "connect_failed"};
  }
  std::string line = cmd + "\n";
  if (::send(sock, line.c_str(), line.size(), 0) < 0) {
    ::close(sock);
    return EventResult{false, Event(), "send_failed"};
  }
  std::string buffer;
  char tmp[512];
  int received_fd = -1;
  while (true) {
    struct msghdr msg {};
    struct iovec iov {};
    iov.iov_base = tmp;
    iov.iov_len = sizeof(tmp);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    std::memset(cmsgbuf, 0, sizeof(cmsgbuf));
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    ssize_t n = ::recvmsg(sock, &msg, 0);
    if (n <= 0) {
      ::close(sock);
      return EventResult{false, Event(), "recv_failed"};
    }
    buffer.append(tmp, tmp + n);
    for (struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
      if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        std::memcpy(&received_fd, CMSG_DATA(cmsg), sizeof(int));
      }
    }
    auto pos = buffer.find('\n');
    if (pos != std::string::npos) {
      std::string resp = buffer.substr(0, pos);
      ::close(sock);
      auto r = parse_response(resp);
      if (!r.ok) {
        if (received_fd >= 0) {
          ::close(received_fd);
        }
        return EventResult{false, Event(), r.error};
      }
      if (received_fd < 0) {
        return EventResult{false, Event(), "no_fd"};
      }
      return EventResult{true, Event(received_fd), ""};
    }
  }
}

Result VirtualDriverTest::adc_adc_0_read() {
  std::ostringstream oss;
  oss << "ADC_READ name=" << "adc_0";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_0_set(double value) {
  std::ostringstream oss;
  oss << "ADC_SET name=" << "adc_0" << " value=" << value;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_0_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=adc name=" << "adc_0";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::adc_adc_0_subscribe(double threshold, Edge edge) {
  std::ostringstream oss;
  oss << "ADC_SUBSCRIBE name=" << "adc_0" << " threshold=" << threshold << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::adc_adc_1_read() {
  std::ostringstream oss;
  oss << "ADC_READ name=" << "adc_1";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_1_set(double value) {
  std::ostringstream oss;
  oss << "ADC_SET name=" << "adc_1" << " value=" << value;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_1_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=adc name=" << "adc_1";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::adc_adc_1_subscribe(double threshold, Edge edge) {
  std::ostringstream oss;
  oss << "ADC_SUBSCRIBE name=" << "adc_1" << " threshold=" << threshold << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::adc_adc_2_read() {
  std::ostringstream oss;
  oss << "ADC_READ name=" << "adc_2";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_2_set(double value) {
  std::ostringstream oss;
  oss << "ADC_SET name=" << "adc_2" << " value=" << value;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_2_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=adc name=" << "adc_2";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::adc_adc_2_subscribe(double threshold, Edge edge) {
  std::ostringstream oss;
  oss << "ADC_SUBSCRIBE name=" << "adc_2" << " threshold=" << threshold << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::adc_adc_3_read() {
  std::ostringstream oss;
  oss << "ADC_READ name=" << "adc_3";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_3_set(double value) {
  std::ostringstream oss;
  oss << "ADC_SET name=" << "adc_3" << " value=" << value;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::adc_adc_3_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=adc name=" << "adc_3";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::adc_adc_3_subscribe(double threshold, Edge edge) {
  std::ostringstream oss;
  oss << "ADC_SUBSCRIBE name=" << "adc_3" << " threshold=" << threshold << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::can_0_write(uint32_t id, const std::string& hexdata) {
  std::ostringstream oss;
  oss << "CAN_SEND board=" << "test" << " name=" << "can_0" << " id=0x" << std::hex << id << " data=" << hexdata;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_0_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_0"
      << " mbox=" << mbox << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_0_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_0"
      << " fifo=" << fifo << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_0_peek_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_0" << " mbox=" << mbox;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_0_peek_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_0" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_0_fifo_len(int fifo) {
  std::ostringstream oss;
  oss << "CAN_FIFO_LEN board=" << "test" << " name=" << "can_0" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::can_0_subscribe_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_0" << " mbox=" << mbox;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_0_subscribe_mbox_range(int start, int end) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_0"
      << " mbox_start=" << start << " mbox_end=" << end;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_0_subscribe_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_0" << " fifo=" << fifo;
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::can_1_write(uint32_t id, const std::string& hexdata) {
  std::ostringstream oss;
  oss << "CAN_SEND board=" << "test" << " name=" << "can_1" << " id=0x" << std::hex << id << " data=" << hexdata;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_1_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_1"
      << " mbox=" << mbox << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_1_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_1"
      << " fifo=" << fifo << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_1_peek_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_1" << " mbox=" << mbox;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_1_peek_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_1" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_1_fifo_len(int fifo) {
  std::ostringstream oss;
  oss << "CAN_FIFO_LEN board=" << "test" << " name=" << "can_1" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::can_1_subscribe_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_1" << " mbox=" << mbox;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_1_subscribe_mbox_range(int start, int end) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_1"
      << " mbox_start=" << start << " mbox_end=" << end;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_1_subscribe_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_1" << " fifo=" << fifo;
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::can_2_write(uint32_t id, const std::string& hexdata) {
  std::ostringstream oss;
  oss << "CAN_SEND board=" << "test" << " name=" << "can_2" << " id=0x" << std::hex << id << " data=" << hexdata;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_2_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_2"
      << " mbox=" << mbox << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_2_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_2"
      << " fifo=" << fifo << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_2_peek_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_2" << " mbox=" << mbox;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_2_peek_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_2" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_2_fifo_len(int fifo) {
  std::ostringstream oss;
  oss << "CAN_FIFO_LEN board=" << "test" << " name=" << "can_2" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::can_2_subscribe_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_2" << " mbox=" << mbox;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_2_subscribe_mbox_range(int start, int end) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_2"
      << " mbox_start=" << start << " mbox_end=" << end;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_2_subscribe_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_2" << " fifo=" << fifo;
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::can_3_write(uint32_t id, const std::string& hexdata) {
  std::ostringstream oss;
  oss << "CAN_SEND board=" << "test" << " name=" << "can_3" << " id=0x" << std::hex << id << " data=" << hexdata;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_3_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_3"
      << " mbox=" << mbox << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_3_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask) {
  std::ostringstream oss;
  oss << "CAN_SET_FILTER board=" << "test" << " name=" << "can_3"
      << " fifo=" << fifo << " filter=0x" << std::hex << filter
      << " mask=0x" << std::hex << mask;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_3_peek_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_3" << " mbox=" << mbox;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_3_peek_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_PEEK board=" << "test" << " name=" << "can_3" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::can_3_fifo_len(int fifo) {
  std::ostringstream oss;
  oss << "CAN_FIFO_LEN board=" << "test" << " name=" << "can_3" << " fifo=" << fifo;
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::can_3_subscribe_mbox(int mbox) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_3" << " mbox=" << mbox;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_3_subscribe_mbox_range(int start, int end) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_3"
      << " mbox_start=" << start << " mbox_end=" << end;
  return send_cmd_with_fd(oss.str());
}

EventResult VirtualDriverTest::can_3_subscribe_fifo(int fifo) {
  std::ostringstream oss;
  oss << "CAN_SUBSCRIBE board=" << "test" << " name=" << "can_3" << " fifo=" << fifo;
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_0_write(bool value) {
  std::ostringstream oss;
  oss << "GPIO_WRITE name=" << "gpio_0" << " client=" << "test" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_0_read() {
  std::ostringstream oss;
  oss << "GPIO_READ name=" << "gpio_0" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_0_clear() {
  std::ostringstream oss;
  oss << "GPIO_CLEAR name=" << "gpio_0" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_0_set_input(bool value) {
  std::ostringstream oss;
  oss << "GPIO_SET_INPUT name=" << "gpio_0" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_0_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=gpio name=" << "gpio_0";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::gpio_gpio_0_subscribe(Edge edge) {
  std::ostringstream oss;
  oss << "GPIO_SUBSCRIBE name=" << "gpio_0" << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_1_write(bool value) {
  std::ostringstream oss;
  oss << "GPIO_WRITE name=" << "gpio_1" << " client=" << "test" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_1_read() {
  std::ostringstream oss;
  oss << "GPIO_READ name=" << "gpio_1" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_1_clear() {
  std::ostringstream oss;
  oss << "GPIO_CLEAR name=" << "gpio_1" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_1_set_input(bool value) {
  std::ostringstream oss;
  oss << "GPIO_SET_INPUT name=" << "gpio_1" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_1_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=gpio name=" << "gpio_1";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::gpio_gpio_1_subscribe(Edge edge) {
  std::ostringstream oss;
  oss << "GPIO_SUBSCRIBE name=" << "gpio_1" << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_2_write(bool value) {
  std::ostringstream oss;
  oss << "GPIO_WRITE name=" << "gpio_2" << " client=" << "test" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_2_read() {
  std::ostringstream oss;
  oss << "GPIO_READ name=" << "gpio_2" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_2_clear() {
  std::ostringstream oss;
  oss << "GPIO_CLEAR name=" << "gpio_2" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_2_set_input(bool value) {
  std::ostringstream oss;
  oss << "GPIO_SET_INPUT name=" << "gpio_2" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_2_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=gpio name=" << "gpio_2";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::gpio_gpio_2_subscribe(Edge edge) {
  std::ostringstream oss;
  oss << "GPIO_SUBSCRIBE name=" << "gpio_2" << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_3_write(bool value) {
  std::ostringstream oss;
  oss << "GPIO_WRITE name=" << "gpio_3" << " client=" << "test" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_3_read() {
  std::ostringstream oss;
  oss << "GPIO_READ name=" << "gpio_3" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_3_clear() {
  std::ostringstream oss;
  oss << "GPIO_CLEAR name=" << "gpio_3" << " client=" << "test";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_3_set_input(bool value) {
  std::ostringstream oss;
  oss << "GPIO_SET_INPUT name=" << "gpio_3" << " value=" << (value ? 1 : 0);
  return send_cmd(oss.str());
}

Result VirtualDriverTest::gpio_gpio_3_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=gpio name=" << "gpio_3";
  return send_cmd(oss.str());
}

EventResult VirtualDriverTest::gpio_gpio_3_subscribe(Edge edge) {
  std::ostringstream oss;
  oss << "GPIO_SUBSCRIBE name=" << "gpio_3" << " edge=" << edge_to_string(edge);
  return send_cmd_with_fd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_0_read() {
  std::ostringstream oss;
  oss << "PWM_READ name=" << "pwm_0";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_0_set(double duty) {
  std::ostringstream oss;
  oss << "PWM_SET name=" << "pwm_0" << " duty=" << duty;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_0_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=pwm name=" << "pwm_0";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_1_read() {
  std::ostringstream oss;
  oss << "PWM_READ name=" << "pwm_1";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_1_set(double duty) {
  std::ostringstream oss;
  oss << "PWM_SET name=" << "pwm_1" << " duty=" << duty;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_1_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=pwm name=" << "pwm_1";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_2_read() {
  std::ostringstream oss;
  oss << "PWM_READ name=" << "pwm_2";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_2_set(double duty) {
  std::ostringstream oss;
  oss << "PWM_SET name=" << "pwm_2" << " duty=" << duty;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_2_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=pwm name=" << "pwm_2";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_3_read() {
  std::ostringstream oss;
  oss << "PWM_READ name=" << "pwm_3";
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_3_set(double duty) {
  std::ostringstream oss;
  oss << "PWM_SET name=" << "pwm_3" << " duty=" << duty;
  return send_cmd(oss.str());
}

Result VirtualDriverTest::pwm_pwm_3_default() {
  std::ostringstream oss;
  oss << "GET_DEFAULT type=pwm name=" << "pwm_3";
  return send_cmd(oss.str());
}


}  // namespace virtual_driver
