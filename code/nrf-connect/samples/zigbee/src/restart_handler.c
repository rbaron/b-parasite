#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zigbee/zigbee_app_utils.h>

#include "restart_handler.h"

LOG_MODULE_REGISTER(restart_handler, CONFIG_LOG_DEFAULT_LEVEL);

static void restart_network_steering_cb(struct k_timer *timer) {
    LOG_DBG("Restart handler expired. Restarting network steering.");
    user_input_indicate();
}

K_TIMER_DEFINE(restart_timer, restart_network_steering_cb, NULL);
