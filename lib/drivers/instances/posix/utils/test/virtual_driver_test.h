#ifndef VIRTUAL_DRIVER_TEST_H
#define VIRTUAL_DRIVER_TEST_H

#include <cstdint>
#include <string>
#include <thread>
#include <utility>

namespace virtual_driver {

struct Result {
  bool ok = false;
  std::string value;
  std::string error;
};

struct EventResult;

enum class Edge {
  Both,
  Rising,
  Falling
};

inline const char* edge_to_string(Edge edge) {
  switch (edge) {
    case Edge::Rising:
      return "rising";
    case Edge::Falling:
      return "falling";
    default:
      return "both";
  }
}

class Event {
 public:
  Event() = default;
  explicit Event(int fd);
  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;
  Event(Event&& other) noexcept;
  Event& operator=(Event&& other) noexcept;
  ~Event();

  bool valid() const;
  int fd() const;
  uint64_t wait();
  bool try_wait(uint64_t& out);

 private:
  int fd_ = -1;
};

using EventHandler = void (*)(uint64_t);

inline std::thread observe(Event& ev, EventHandler handler) {
  return std::thread([&ev, handler]() {
    while (ev.valid()) {
      auto n = ev.wait();
      if (n == 0) {
        break;
      }
      handler(n);
    }
  });
}

template <typename F>
inline std::thread observe(Event& ev, F&& handler) {
  return std::thread([&ev, fn = std::forward<F>(handler)]() mutable {
    while (ev.valid()) {
      auto n = ev.wait();
      if (n == 0) {
        break;
      }
      fn(n);
    }
  });
}

struct EventResult {
  bool ok = false;
  Event event;
  std::string error;
};

class VirtualDriverTest {
 public:
  explicit VirtualDriverTest(std::string socket_path);

  Result adc_adc_0_read();
  Result adc_adc_0_set(double value);
  Result adc_adc_0_default();
  EventResult adc_adc_0_subscribe(double threshold, Edge edge = Edge::Both);
  Result adc_adc_1_read();
  Result adc_adc_1_set(double value);
  Result adc_adc_1_default();
  EventResult adc_adc_1_subscribe(double threshold, Edge edge = Edge::Both);
  Result adc_adc_2_read();
  Result adc_adc_2_set(double value);
  Result adc_adc_2_default();
  EventResult adc_adc_2_subscribe(double threshold, Edge edge = Edge::Both);
  Result adc_adc_3_read();
  Result adc_adc_3_set(double value);
  Result adc_adc_3_default();
  EventResult adc_adc_3_subscribe(double threshold, Edge edge = Edge::Both);
  Result can_0_write(uint32_t id, const std::string& hexdata);
  Result can_0_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask);
  Result can_0_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask);
  Result can_0_peek_mbox(int mbox);
  Result can_0_peek_fifo(int fifo);
  Result can_0_fifo_len(int fifo);
  EventResult can_0_subscribe_mbox(int mbox);
  EventResult can_0_subscribe_mbox_range(int start, int end);
  EventResult can_0_subscribe_fifo(int fifo);
  Result can_1_write(uint32_t id, const std::string& hexdata);
  Result can_1_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask);
  Result can_1_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask);
  Result can_1_peek_mbox(int mbox);
  Result can_1_peek_fifo(int fifo);
  Result can_1_fifo_len(int fifo);
  EventResult can_1_subscribe_mbox(int mbox);
  EventResult can_1_subscribe_mbox_range(int start, int end);
  EventResult can_1_subscribe_fifo(int fifo);
  Result can_2_write(uint32_t id, const std::string& hexdata);
  Result can_2_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask);
  Result can_2_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask);
  Result can_2_peek_mbox(int mbox);
  Result can_2_peek_fifo(int fifo);
  Result can_2_fifo_len(int fifo);
  EventResult can_2_subscribe_mbox(int mbox);
  EventResult can_2_subscribe_mbox_range(int start, int end);
  EventResult can_2_subscribe_fifo(int fifo);
  Result can_3_write(uint32_t id, const std::string& hexdata);
  Result can_3_set_filter_mbox(int mbox, uint32_t filter, uint32_t mask);
  Result can_3_set_filter_fifo(int fifo, uint32_t filter, uint32_t mask);
  Result can_3_peek_mbox(int mbox);
  Result can_3_peek_fifo(int fifo);
  Result can_3_fifo_len(int fifo);
  EventResult can_3_subscribe_mbox(int mbox);
  EventResult can_3_subscribe_mbox_range(int start, int end);
  EventResult can_3_subscribe_fifo(int fifo);
  Result gpio_gpio_0_write(bool value);
  Result gpio_gpio_0_read();
  Result gpio_gpio_0_clear();
  Result gpio_gpio_0_set_input(bool value);
  Result gpio_gpio_0_default();
  EventResult gpio_gpio_0_subscribe(Edge edge = Edge::Both);
  Result gpio_gpio_1_write(bool value);
  Result gpio_gpio_1_read();
  Result gpio_gpio_1_clear();
  Result gpio_gpio_1_set_input(bool value);
  Result gpio_gpio_1_default();
  EventResult gpio_gpio_1_subscribe(Edge edge = Edge::Both);
  Result gpio_gpio_2_write(bool value);
  Result gpio_gpio_2_read();
  Result gpio_gpio_2_clear();
  Result gpio_gpio_2_set_input(bool value);
  Result gpio_gpio_2_default();
  EventResult gpio_gpio_2_subscribe(Edge edge = Edge::Both);
  Result gpio_gpio_3_write(bool value);
  Result gpio_gpio_3_read();
  Result gpio_gpio_3_clear();
  Result gpio_gpio_3_set_input(bool value);
  Result gpio_gpio_3_default();
  EventResult gpio_gpio_3_subscribe(Edge edge = Edge::Both);
  Result pwm_pwm_0_read();
  Result pwm_pwm_0_set(double duty);
  Result pwm_pwm_0_default();
  Result pwm_pwm_1_read();
  Result pwm_pwm_1_set(double duty);
  Result pwm_pwm_1_default();
  Result pwm_pwm_2_read();
  Result pwm_pwm_2_set(double duty);
  Result pwm_pwm_2_default();
  Result pwm_pwm_3_read();
  Result pwm_pwm_3_set(double duty);
  Result pwm_pwm_3_default();

 private:
  std::string socket_path_;
  Result send_cmd(const std::string& cmd);
  EventResult send_cmd_with_fd(const std::string& cmd);
};

};  // namespace virtual_driver

#endif
