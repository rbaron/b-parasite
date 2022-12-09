#ifndef _PRST_ADC_H_
#define _PRST_ADC_H_

#include <stdint.h>

typedef struct {
  int16_t raw;
  int32_t millivolts;
  float voltage;
} prst_adc_read_t;

typedef struct {
  prst_adc_read_t adc_read;
  float percentage;
} prst_batt_t;

typedef struct {
  prst_adc_read_t adc_read;
  // A value from 0 (completely dry) to 2^10 (completely wet).
  uint16_t relative;
  // In [0, 1.0].
  float percentage;
} prst_adc_soil_moisture_t;

typedef struct prst_adc_photo_sensor {
  prst_adc_read_t adc_read;
  // Value in lux.
  uint16_t brightness;
} prst_adc_photo_sensor_t;

int prst_adc_init();

int prst_adc_batt_read(prst_batt_t* out);

int prst_adc_soil_read(float battery_voltage, prst_adc_soil_moisture_t* out);

int prst_adc_photo_read(float battery_voltage, prst_adc_photo_sensor_t* out);

#endif  // _PRST_ADC_H_