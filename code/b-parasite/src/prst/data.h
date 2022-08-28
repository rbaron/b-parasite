#ifndef _PRST_DATA_H_
#define _PRST_DATA_H_

#include <stdint.h>

typedef struct {
  uint16_t batt_mv;
  float temp_c;
  uint16_t humi;
  uint16_t soil_moisture;
  uint16_t lux;
  uint8_t run_counter;
} prst_sensor_data_t;

#endif  // _PRST_DATA_H_