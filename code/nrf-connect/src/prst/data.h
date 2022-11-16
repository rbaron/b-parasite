#ifndef _PRST_DATA_H_
#define _PRST_DATA_H_

#include "prst/adc.h"
#include "prst/shtc3.h"

typedef struct {
  prst_adc_soil_moisture_t soil;
  prst_adc_photo_sensor_t photo;
  prst_adc_read_t batt;
  prst_shtc3_read_t shtc3;
} prst_sensors_t;

#endif  // _PRST_DATA_H_
