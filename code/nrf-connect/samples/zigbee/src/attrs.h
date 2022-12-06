#ifndef _PRST_ZB_ATTRS_H_
#define _PRST_ZB_ATTRS_H_

#include <zboss_api.h>

// Relative humimdity cluster - section 4.7.2.2.1.
typedef struct {
  // 100x relative humidity (mandatory).
  zb_uint16_t rel_humidity;
  // 0x0000 – 0x270f (mandatory).
  zb_uint16_t min_val;
  // 0x0001 – 0x2710 (mandatory).
  zb_uint16_t max_val;
} prst_rel_humidity_attrs_t;

// Power configuration cluster - section 3.3.2.2.3
typedef struct {
  // Units of 100 mV. 0x00 - 0xff (optional, not reportable :()).
  zb_uint8_t voltage;
  // Units of 0.5%. 0x00 (0%) - 0xc8 (100%) (optional, reportable).
  zb_uint8_t percentage;
  // zb_uint8_t quantity;
  // zb_uint8_t size;
  // zb_uint8_t rated_voltage;
  // zb_uint8_t voltage_min_thres;
  // zb_uint8_t percentage_min_thres;
} prst_batt_attrs_t;

// Soil moisture cluster.
typedef struct {
  zb_uint16_t percentage;
} prst_soil_moisture_attrs_t;

#endif  // _PRST_ZB_ATTRS_H_
