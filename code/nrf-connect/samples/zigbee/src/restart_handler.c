#include "restart_handler.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>

LOG_MODULE_REGISTER(restart_handler, CONFIG_LOG_DEFAULT_LEVEL);

static void restart_network_steering_cb(struct k_timer *timer) {
  LOG_DBG("Restart handler expired. Restarting network steering.");
  // If the device is not commissioned, the rejoin procedure is started.
  user_input_indicate();
}

K_TIMER_DEFINE(restart_timer, restart_network_steering_cb, NULL);

static void prst_restart_watchdog_start() {
  k_timer_start(&restart_timer, K_SECONDS(PRST_ZB_RESET_TIMEOUT), K_MSEC(0));
}

static void prst_restart_watchdog_stop() {
  k_timer_stop(&restart_timer);
}
