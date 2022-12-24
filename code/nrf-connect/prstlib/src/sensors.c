#include "prstlib/sensors.h"

#include <zephyr/logging/log.h>

#include "prstlib/adc.h"
#include "prstlib/led.h"
#include "prstlib/macros.h"

LOG_MODULE_REGISTER(sensors, CONFIG_PRSTLIB_LOG_LEVEL);

int prst_sensors_read_all(prst_sensors_t *sensors) {
  RET_IF_ERR(prst_adc_batt_read(&sensors->batt));
  RET_IF_ERR(prst_adc_soil_read(sensors->batt.adc_read.voltage, &sensors->soil));
  RET_IF_ERR(prst_adc_photo_read(sensors->batt.adc_read.voltage, &sensors->photo));
  RET_IF_ERR(prst_shtc3_read(&sensors->shtc3))

  LOG_DBG("Batt: %d mV (%.2f%%)", sensors->batt.adc_read.millivolts,
          100 * sensors->batt.percentage);
  LOG_DBG("Soil: %.0f %% (%d mV)", 100 * sensors->soil.percentage,
          sensors->soil.adc_read.millivolts);
  LOG_DBG("Photo: %u lx (%d mV)", sensors->photo.brightness,
          sensors->soil.adc_read.millivolts);
  LOG_DBG("Temp: %f oC", sensors->shtc3.temp_c);
  LOG_DBG("Humi: %.0f %%", 100 * sensors->shtc3.rel_humi);
  LOG_DBG("--------------------------------------------------");

  return 0;
}