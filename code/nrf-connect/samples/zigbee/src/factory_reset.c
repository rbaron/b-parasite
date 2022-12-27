#include "factory_reset.h"

#include <hal/nrf_power.h>
#include <prstlib/led.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>

#include "double_reset_detector.h"

LOG_MODULE_REGISTER(factory_reset, CONFIG_LOG_DEFAULT_LEVEL);

static int factory_reset() {
  // TODO: consider zb_bdb_reset_via_local_action(/*param=*/0);
  zigbee_erase_persistent_storage(/*erase=*/true);
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

int prst_zb_factory_reset_check() {
#if CONFIG_PRST_ZB_FACTORY_RESET_VIA_DOUBLE_RESET
  return prst_detect_double_reset(double_reset_handler);
#elif CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN
  return factory_reset_if_reset_via_reset_pin();
#endif  // CONFIG_PRST_ZB_FACTORY_RESET_VIA_RESET_PIN
}