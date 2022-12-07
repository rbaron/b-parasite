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
} prst_batt_attrs_t;

// Soil moisture cluster.
typedef struct {
  // 0-100, units of 0.01?
  zb_uint16_t percentage;
} prst_soil_moisture_attrs_t;

struct zb_device_ctx {
  zb_zcl_basic_attrs_ext_t basic_attr;
  zb_zcl_identify_attrs_t identify_attr;
  // In units of 0.01 C.
  zb_zcl_temp_measurement_attrs_t temp_measure_attrs;
  prst_rel_humidity_attrs_t rel_humidity_attrs;
  // In units of 100 mV.
  prst_batt_attrs_t batt_attrs;
  prst_soil_moisture_attrs_t soil_moisture_attrs;
};

#endif  // _PRST_ZB_ATTRS_H_
