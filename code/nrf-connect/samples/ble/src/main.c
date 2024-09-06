#include <prstlib/adc.h>
#include <prstlib/button.h>
#include <prstlib/led.h>
#include <prstlib/macros.h>
#include <prstlib/sensors.h>
#include <prstlib/shtc3.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>

#include "ble.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static int prst_init() {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());
  RET_IF_ERR(prst_ble_init());
  return 0;
}

static int prst_loop(prst_sensors_t *sensors) {
  RET_IF_ERR(prst_sensors_read_all(sensors));
  RET_IF_ERR(prst_ble_adv_set_data(sensors));
  RET_IF_ERR(prst_ble_adv_start());
  k_msleep(CONFIG_PRST_BLE_ADV_DURATION_MSEC);
  RET_IF_ERR(prst_ble_adv_stop());
  return 0;
}

int main(void) {
  __ASSERT(!prst_init(), "Error in prst_init()");
  prst_led_flash(2);
  prst_sensors_t sensors;
  while (true) {
    __ASSERT(!prst_loop(&sensors), "Error in prst_loop()");
    k_msleep(CONFIG_PRST_SLEEP_DURATION_MSEC);
  }
}
