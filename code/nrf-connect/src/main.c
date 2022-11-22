#include <logging/log.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/zephyr.h>

#include "prst/adc.h"
#include "prst/ble/ble.h"
#include "prst/button.h"
#include "prst/data.h"
#include "prst/led.h"
#include "prst/macros.h"
#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());
  RET_IF_ERR(prst_ble_init());

  RET_IF_ERR(prst_led_flash(2));

  prst_sensors_t sensors;
  while (true) {
    RET_IF_ERR(prst_adc_batt_read(&sensors.batt));
    RET_IF_ERR(prst_adc_soil_read(sensors.batt.voltage, &sensors.soil));
    RET_IF_ERR(prst_adc_photo_read(sensors.batt.voltage, &sensors.photo));
    RET_IF_ERR(prst_shtc3_read(&sensors.shtc3))

    LOG_INF("Batt: %d mV", sensors.batt.millivolts);
    LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * sensors.soil.percentage,
            sensors.soil.adc_read.voltage);
    LOG_INF("Photo: %u lx (%.3f mV)", sensors.photo.brightness,
            sensors.soil.adc_read.voltage);
    LOG_INF("Temp: %f oC", sensors.shtc3.temp_c);
    LOG_INF("Humi: %.0f %%", 100.0 * sensors.shtc3.rel_humi);
    LOG_INF("--------------------------------------------------");

    RET_IF_ERR(prst_ble_adv_set_data(&sensors));
    RET_IF_ERR(prst_ble_adv_start());

    k_sleep(K_SECONDS(2));

    RET_IF_ERR(prst_ble_adv_stop());

    k_sleep(K_SECONDS(2));

    prst_led_flash(1);

    // Example: go to deep sleep.
    pm_state_force(0u, &(struct pm_state_info){PM_STATE_SOFT_OFF, 0, 0});
    k_sleep(K_SECONDS(2));
  }
}
