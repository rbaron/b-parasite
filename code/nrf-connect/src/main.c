#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/adc.h"
#include "prst/led.h"
#include "prst/macros.h"
#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());

  prst_adc_read_t batt;
  prst_adc_soil_moisture_t soil;
  prst_adc_photo_sensor_t photo;
  while (true) {
    RET_IF_ERR(prst_adc_batt_read(&batt));
    RET_IF_ERR(prst_adc_soil_read(batt.voltage, &soil));
    RET_IF_ERR(prst_adc_photo_read(batt.voltage, &photo));

    LOG_INF("Batt: %d mV", batt.millivolts);
    LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * soil.percentage,
            soil.adc_read.voltage);
    LOG_INF("Photo: %u lx (%.3f mV)", photo.brightness, soil.adc_read.voltage);

    prst_led_flash(3);

    k_msleep(500);
  }
}
