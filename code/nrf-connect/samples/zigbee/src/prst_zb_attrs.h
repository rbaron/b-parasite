#ifndef _PRST_ZB_ATTRS_H_
#define _PRST_ZB_ATTRS_H_

#include <zboss_api_addons.h>

// Relative humimdity cluster - section 4.7.2.2.1.
typedef struct {
  // 100x relative humidity (mandatory).
  zb_uint16_t rel_humidity;
  // 0x0000 – 0x270f (mandatory).
  zb_uint16_t min_val;
  // 0x0001 – 0x2710 (mandatory).
  zb_uint16_t max_val;
} prst_rel_humidity_attrs_t;

// Power configuration cluster - section 3.3.2.2.3.
typedef struct {
  // Units of 100 mV. 0x00 - 0xff (optional, not reportable :()).
  zb_uint8_t voltage;
  // Units of 0.5%. 0x00 (0%) - 0xc8 (100%) (optional, reportable).
  zb_uint8_t percentage;
  // Whole number of battery cells used to power device
  zb_uint8_t quantity;
  // Enumeration of battery size
  enum zb_zcl_power_config_battery_size_e size;
} prst_batt_attrs_t;

// Soil moisture cluster.
typedef struct {
  // 0-100, units of 0.01?
  zb_uint16_t percentage;
} prst_soil_moisture_attrs_t;

// Illuminance cluster - section 4.2.2.2.
typedef struct {
  // 10000 * log_10(lux) + 1.
  zb_uint16_t log_lux;
} prst_illuminancce_attrs_t;

struct zb_device_ctx {
  zb_zcl_basic_attrs_ext_t basic_attr;
  zb_zcl_identify_attrs_t identify_attr;
  zb_zcl_temp_measurement_attrs_t temp_measure_attrs;
  prst_rel_humidity_attrs_t rel_humidity_attrs;
  prst_batt_attrs_t batt_attrs;
  prst_soil_moisture_attrs_t soil_moisture_attrs;
  prst_illuminancce_attrs_t illuminance_attrs;
};

void prst_zb_attrs_init(struct zb_device_ctx *dev_ctx);

zb_zcl_status_t prst_zb_set_attr_value(zb_uint16_t cluster_id, zb_uint16_t attr_id, void *data);

#endif  // _PRST_ZB_ATTRS_H_
