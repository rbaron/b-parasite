#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/adc.h"
#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void main(void) {
  if (prst_adc_init() != 0) {
    LOG_ERR("Error initializing ADC.");
  }

  while (true) {
    prst_adc_read_t batt = prst_adc_batt_read();
    prst_adc_soil_moisture_t soil = prst_adc_soil_read(batt.voltage);

    // LOG_INF("Batt: %d mV", batt.millivolts);
    LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * soil.percentage,
            soil.adc_read.voltage);

    k_msleep(500);
  }
}
