#include <logging/log.h>
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

  RET_IF_ERR(prst_ble_adv_start());

  prst_sensors_t sensors;

  while (true) {
    RET_IF_ERR(prst_adc_batt_read(&sensors.batt));
    RET_IF_ERR(prst_adc_soil_read(sensors.batt.voltage, &sensors.soil));
    RET_IF_ERR(prst_adc_photo_read(sensors.batt.voltage, &sensors.photo));

    LOG_INF("Batt: %d mV", sensors.batt.millivolts);
    LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * sensors.soil.percentage,
            sensors.soil.adc_read.voltage);
    LOG_INF("Photo: %u lx (%.3f mV)", sensors.photo.brightness,
            sensors.soil.adc_read.voltage);
    k_msleep(500);
  }
}