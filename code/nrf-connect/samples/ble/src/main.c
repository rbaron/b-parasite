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

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());
  RET_IF_ERR(prst_ble_init());

  RET_IF_ERR(prst_led_flash(2));

  prst_sensors_t sensors;
  while (true) {
    RET_IF_ERR(prst_sensors_read_all(&sensors));

    RET_IF_ERR(prst_ble_adv_set_data(&sensors));
    RET_IF_ERR(prst_ble_adv_start());

    k_sleep(K_SECONDS(CONFIG_PRST_BLE_ADV_DURATION_SEC));

    RET_IF_ERR(prst_ble_adv_stop());

    k_sleep(K_SECONDS(CONFIG_PRST_SLEEP_DURATION_SEC));
  }
}
