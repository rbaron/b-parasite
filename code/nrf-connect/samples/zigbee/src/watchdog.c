#include "watchdog.h"

#include <prstlib/macros.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define PRST_ZB_WATCHDOG_TIMEOUT_SEC (2 * CONFIG_PRST_ZB_SLEEP_DURATION_SEC)

LOG_MODULE_REGISTER(watchdog, CONFIG_LOG_DEFAULT_LEVEL);

typedef struct {
  const struct device *const wdt;
  int wtd_channel_id;
} wtd_data_t;

static wtd_data_t wtd_data = {
    .wdt = DEVICE_DT_GET(DT_NODELABEL(wdt)),
};

int prst_watchdog_start() {
  static const struct wdt_timeout_cfg wdt_settings = {
      .window = {
          .min = 0,
          .max = PRST_ZB_WATCHDOG_TIMEOUT_SEC * MSEC_PER_SEC,
      },
      // NULL callback means use the default, which is the system reset.
      .callback = NULL,
      .flags = WDT_FLAG_RESET_SOC};

  RET_IF_ERR(!device_is_ready(wtd_data.wdt));

  // Install timeout.
  wtd_data.wtd_channel_id = wdt_install_timeout(wtd_data.wdt, &wdt_settings);
  RET_CHECK(wtd_data.wtd_channel_id >= 0, "Failed to install watchdog timeout.");

  // Start watchdog.
  RET_IF_ERR(wdt_setup(wtd_data.wdt, WDT_OPT_PAUSE_HALTED_BY_DBG));

  return 0;
}

int prst_watchdog_feed() {
  RET_IF_ERR(wdt_feed(wtd_data.wdt, wtd_data.wtd_channel_id));
  return 0;
}
