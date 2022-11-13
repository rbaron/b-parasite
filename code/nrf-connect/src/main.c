#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/adc.h"
#include "prst/macros.h"
#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void) {
  if (prst_adc_init() != 0) {
    LOG_ERR("Error initializing ADC.");
  }

  prst_adc_read_t batt;
  prst_adc_soil_moisture_t soil;
  prst_adc_photo_sensor_t photo;
  while (true) {
    RET_IF_ERR(prst_adc_batt_read(&batt));
    RET_IF_ERR(prst_adc_soil_read(batt.voltage, &soil));
    RET_IF_ERR(prst_adc_photo_read(batt.voltage, &photo));

    // LOG_INF("Batt: %d mV", batt.millivolts);
    // LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * soil.percentage,
    //         soil.adc_read.voltage);
    LOG_INF("Photo: %u lx (%.3f mV)", photo.brightness, soil.adc_read.voltage);

    k_msleep(500);
  }
}
