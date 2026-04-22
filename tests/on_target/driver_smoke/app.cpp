#include "smoke_test.hpp"

namespace {
smoke::SmokeContext g_context{};
bool g_started{false};
}  // namespace

void app_start(void) {
  if (g_started) {
    return;
  }
  g_started = true;

  const auto usb_status = g_context.usb.init();
  g_context.usb_ready = usb_status == ru::driver::result::OK;
  smoke::mark_result(g_context, usb_status, smoke::k_error_usb_init);
  smoke::usb_line(g_context, "driver-smoke: boot\r\n");

  smoke::start_freertos_smoke(g_context);
  smoke::start_adc_smoke(g_context);
  smoke::start_can_smoke(g_context);
  smoke::run_eeprom_smoke(g_context);
  smoke::can_report_eeprom(g_context);
  smoke::start_report_smoke(g_context);

  smoke::usb_line(g_context, "driver-smoke: scheduler starting\r\n");
}
