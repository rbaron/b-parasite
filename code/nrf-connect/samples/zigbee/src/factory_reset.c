#include "factory_reset.h"

#include <hal/nrf_power.h>
#include <prstlib/button.h>
#include <prstlib/led.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>

#include "double_reset_detector.h"

LOG_MODULE_REGISTER(factory_reset, CONFIG_LOG_DEFAULT_LEVEL);

static int factory_reset() {
  LOG_WRN("Factory resetting device");
  zb_bdb_reset_via_local_action(/*param=*/0);
  return 0;
}

#if CONFIG_PRST_ZB_FACTORY_RESET_VIA_DOUBLE_RESET
int double_reset_handler() {
  LOG_WRN("Called double reset handler");
  prst_led_flash(5);
  return factory_reset();
}
#endif  // CONFIG_PRST_ZB_FACTORY_RESET_VIA_DOUBLE_RESET

#if CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN
static int factory_reset_if_reset_via_reset_pin() {
  uint32_t reset_reason = nrf_power_resetreas_get(NRF_POWER);
  // If we're resetting via the RESET pin (e.g.: reset pin shorting, firmware flashing).
  if (reset_reason & 0x1) {
    LOG_WRN("Manual reset / re-flashing detected - erasing pairing info");
    return factory_reset();
  } else {  // It's a power-on cycle (e.g.: swapping battery, first boot).
    LOG_INF("Power-on cycle - keeping pairing info");
  }
  return 0;
}
#endif  // CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN

#if CONFIG_PRST_ZB_FACTORY_RESET_VIA_SW1
static void timer_do_reset(zb_uint8_t unused_param) {
  LOG_WRN("SW1 button was pressed for 5 seconds, factory resetting device");
  prst_led_flash(/*times=*/5);
  factory_reset();
}

static void sw1_factory_reset_check_timer_cb(struct k_timer *timer_id) {
  if (!prst_button_poll(PRST_BUTTON_SW1)) {
    LOG_DBG("SW1 button was released, will not factory reset device");
    return;
  }
  ZB_SCHEDULE_APP_CALLBACK(timer_do_reset, /*param=*/0);
}

K_TIMER_DEFINE(sw1_factory_reset_check_timer, sw1_factory_reset_check_timer_cb, NULL);
#endif

int prst_zb_factory_reset_check() {
#if CONFIG_PRST_ZB_FACTORY_RESET_VIA_DOUBLE_RESET
  return prst_detect_double_reset(double_reset_handler);
#elif CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN
  return factory_reset_if_reset_via_reset_pin();
#elif CONFIG_PRST_ZB_FACTORY_RESET_VIA_SW1
  if (prst_button_poll(PRST_BUTTON_SW1)) {
    LOG_DBG("SW1 pressed. Scheduling timer");
    k_timer_start(&sw1_factory_reset_check_timer, K_SECONDS(5), K_NO_WAIT);
  }
  return 0;
#elif CONFIG_PRST_ZB_FACTORY_RESET_DISABLED
  return 0;
#else
#error "No factory reset method selected -- explicitly select CONFIG_PRST_ZB_FACTORY_RESET_DISABLED=y to disable it"
#endif  // CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN
}