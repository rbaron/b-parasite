#ifndef _PRST_ADC_H_
#define _PRST_ADC_H_

#include <stdint.h>

typedef struct prst_adc_batt_val {
  int16_t raw;
  uint16_t millivolts;
  double voltage;
} prst_adc_batt_read_t;

void prst_adc_init();

prst_adc_batt_read_t prst_adc_batt_read();

int16_t prst_adc_soil_read();

#endif  // _PRST_ADC_H_