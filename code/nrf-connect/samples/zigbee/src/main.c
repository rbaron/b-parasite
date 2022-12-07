/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @brief Zigbee application template.
 */

#include <dk_buttons_and_leds.h>
#include <prstlib/adc.h>
#include <prstlib/button.h>
#include <prstlib/led.h>
#include <prstlib/macros.h>
#include <prstlib/sensors.h>
#include <prstlib/shtc3.h>
#include <zb_nrf_platform.h>
#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zcl/zb_zcl_power_config.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>

#include "prst_zb_attrs.h"
#include "prst_zb_endpoint_defs.h"
#include "prst_zb_soil_moisture_defs.h"

#define IDENTIFY_MODE_BUTTON DK_BTN4_MSK
#define FACTORY_RESET_BUTTON IDENTIFY_MODE_BUTTON

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

static struct zb_device_ctx dev_ctx;

static prst_sensors_t sensors;

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(
    identify_attr_list,
    &dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(
    basic_attr_list,
    &dev_ctx.basic_attr.zcl_version,
    &dev_ctx.basic_attr.app_version,
    &dev_ctx.basic_attr.stack_version,
    &dev_ctx.basic_attr.hw_version,
    dev_ctx.basic_attr.mf_name,
    dev_ctx.basic_attr.model_id,
    dev_ctx.basic_attr.date_code,
    &dev_ctx.basic_attr.power_source,
    dev_ctx.basic_attr.location_id,
    &dev_ctx.basic_attr.ph_env,
    dev_ctx.basic_attr.sw_ver);

ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(temp_measurement_attr_list,
                                            &dev_ctx.temp_measure_attrs.measure_value,
                                            &dev_ctx.temp_measure_attrs.min_measure_value,
                                            &dev_ctx.temp_measure_attrs.max_measure_value,
                                            &dev_ctx.temp_measure_attrs.tolerance);

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(
    rel_humi_attr_list,
    &dev_ctx.rel_humidity_attrs.rel_humidity,
    &dev_ctx.rel_humidity_attrs.min_val,
    &dev_ctx.rel_humidity_attrs.max_val);

// https://devzone.nordicsemi.com/f/nordic-q-a/85315/zboss-declare-power-config-attribute-list-for-battery-bat_num
#define bat_num
ZB_ZCL_DECLARE_POWER_CONFIG_BATTERY_ATTRIB_LIST_EXT(
    batt_attr_list,
    &dev_ctx.batt_attrs.voltage,
    /*battery_size=*/NULL,
    /*battery_quantity=*/NULL,
    /*battery_rated_voltage=*/NULL,
    /*battery_alarm_mask=*/NULL,
    /*battery_voltage_min_threshold=*/NULL,
    /*battery_percentage_remaining=*/&dev_ctx.batt_attrs.percentage,
    /*battery_voltage_threshold1=*/NULL,
    /*battery_voltage_threshold2=*/NULL,
    /*battery_voltage_threshold3=*/NULL,
    /*battery_percentage_min_threshold=*/NULL,
    /*battery_percentage_threshold1=*/NULL,
    /*battery_percentage_threshold2=*/NULL,
    /*battery_percentage_threshold3=*/NULL,
    /*battery_alarm_state=*/NULL);

PRST_ZB_ZCL_DECLARE_SOIL_MOISTURE_ATTRIB_LIST(
    soil_moisture_attr_list,
    &dev_ctx.soil_moisture_attrs.percentage);

ZB_DECLARE_RANGE_EXTENDER_CLUSTER_LIST(
    app_template_clusters,
    basic_attr_list,
    identify_attr_list,
    temp_measurement_attr_list,
    rel_humi_attr_list,
    basic_attr_list,
    soil_moisture_attr_list);

ZB_DECLARE_RANGE_EXTENDER_EP(
    app_template_ep,
    PRST_ZIGBEE_ENDPOINT,
    app_template_clusters);

ZBOSS_DECLARE_DEVICE_CTX_1_EP(
    app_template_ctx,
    app_template_ep);

static void app_clusters_attr_init(void) {
  dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
  dev_ctx.basic_attr.power_source = ZB_ZCL_BASIC_POWER_SOURCE_BATTERY;
  ZB_ZCL_SET_STRING_VAL(
      dev_ctx.basic_attr.mf_name,
      PRST_BASIC_MANUF_NAME,
      ZB_ZCL_STRING_CONST_SIZE(PRST_BASIC_MANUF_NAME));

  ZB_ZCL_SET_STRING_VAL(
      dev_ctx.basic_attr.model_id,
      PRST_BASIC_MODEL_ID,
      ZB_ZCL_STRING_CONST_SIZE(PRST_BASIC_MODEL_ID));

  dev_ctx.identify_attr.identify_time =
      ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
}

static void toggle_identify_led(zb_bufid_t bufid) {
  static int blink_status;
  ZB_SCHEDULE_APP_ALARM(toggle_identify_led, bufid, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
}

static void identify_cb(zb_bufid_t bufid) {
  zb_ret_t zb_err_code;

  if (bufid) {
    ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
  } else {
    zb_err_code = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led, ZB_ALARM_ANY_PARAM);
    ZVUNUSED(zb_err_code);
  }
}

static void start_identifying(zb_bufid_t bufid) {
  ZVUNUSED(bufid);

  if (ZB_JOINED()) {
    if (dev_ctx.identify_attr.identify_time ==
        ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {
      zb_ret_t zb_err_code = zb_bdb_finding_binding_target(
          PRST_ZIGBEE_ENDPOINT);

      if (zb_err_code == RET_OK) {
        LOG_INF("Enter identify mode");
      } else if (zb_err_code == RET_INVALID_STATE) {
        LOG_WRN("RET_INVALID_STATE - Cannot enter identify mode");
      } else {
        ZB_ERROR_CHECK(zb_err_code);
      }
    } else {
      LOG_INF("Cancel identify mode");
      zb_bdb_finding_binding_target_cancel();
    }
  } else {
    LOG_WRN("Device not in a network - cannot enter identify mode");
  }
}

static void button_changed(uint32_t button_state, uint32_t has_changed) {
  if (IDENTIFY_MODE_BUTTON & has_changed) {
    if (IDENTIFY_MODE_BUTTON & button_state) {
    } else {
      if (was_factory_reset_done()) {
        LOG_DBG("After Factory Reset - ignore button release");
      } else {
        ZB_SCHEDULE_APP_CALLBACK(start_identifying, 0);
      }
    }
  }

  check_factory_reset_button(button_state, has_changed);
}

void zboss_signal_handler(zb_bufid_t bufid) {
  ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
  if (bufid) {
    zb_buf_free(bufid);
  }
}

void update_sensors_cb(zb_uint8_t arg) {
  LOG_INF("Updating sensors");

  if (prst_sensors_read_all(&sensors)) {
    LOG_ERR("Unable to read sensors");
    return;
  }

  static zb_uint8_t batt = 10;
  batt += 1;
  zb_zcl_set_attr_val(PRST_ZIGBEE_ENDPOINT, ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                      ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
                      (zb_uint8_t*)&batt, ZB_FALSE);

  static zb_uint8_t batt_percentage = 10;
  batt_percentage += 1;
  zb_zcl_set_attr_val(PRST_ZIGBEE_ENDPOINT, ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                      ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
                      (zb_uint8_t*)&batt_percentage, ZB_FALSE);

  static zb_int16_t temperature_value = 27;
  temperature_value += 1;
  zb_zcl_set_attr_val(PRST_ZIGBEE_ENDPOINT, ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                      ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
                      (zb_uint8_t*)&temperature_value, ZB_FALSE);

  static zb_int16_t rel_humi = 12;
  rel_humi += 1;
  zb_zcl_set_attr_val(PRST_ZIGBEE_ENDPOINT, ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
                      ZB_ZCL_CLUSTER_SERVER_ROLE, ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
                      (zb_uint8_t*)&rel_humi, ZB_FALSE);

  zb_int16_t soil_moisture = 100 * 100 * sensors.soil.percentage;
  zb_zcl_set_attr_val(PRST_ZIGBEE_ENDPOINT, PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID,
                      ZB_ZCL_CLUSTER_SERVER_ROLE, PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID,
                      (zb_uint8_t*)&soil_moisture, ZB_FALSE);

  ZB_SCHEDULE_APP_ALARM(update_sensors_cb, NULL, ZB_TIME_ONE_SECOND * 1);
}

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());

  register_factory_reset_button(FACTORY_RESET_BUTTON);

  app_clusters_attr_init();

  ZB_AF_REGISTER_DEVICE_CTX(&app_template_ctx);

  ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(PRST_ZIGBEE_ENDPOINT, identify_cb);

  update_sensors_cb(/*arg=*/0);

  // zigbee_configure_sleepy_behavior(/*enable=*/true);

  RET_IF_ERR(prst_led_flash(2));

  zigbee_enable();

  LOG_INF("Zigbee application template started");

  return 0;
}
