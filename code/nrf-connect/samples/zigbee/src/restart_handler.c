#include "restart_handler.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>

#include "debug_counters.h"

LOG_MODULE_REGISTER(restart_handler, CONFIG_LOG_DEFAULT_LEVEL);

void callback_work_handler(struct k_work *work) {
  LOG_INF("Running restart callback_work_handler.");
  prst_debug_counters_increment("steering_watchdog_restart");
  // If the device is not commissioned, the rejoin procedure is started.
  user_input_indicate();
}

K_WORK_DEFINE(callback_work, callback_work_handler);

// Runs in an ISR context. We offload the actual work to a workqueue.
static void restart_network_steering_cb(struct k_timer *timer) {
  LOG_INF("Triggered restart_network_steering_cb. Offloading work.");
  k_work_submit(&callback_work);
}

K_TIMER_DEFINE(restart_timer, restart_network_steering_cb, NULL);

void prst_restart_watchdog_start() {
  k_timer_start(&restart_timer, K_SECONDS(CONFIG_PRST_ZB_RESTART_WATCHDOG_TIMEOUT_SEC), K_MSEC(0));
}

void prst_restart_watchdog_stop() {
  k_timer_stop(&restart_timer);
}
