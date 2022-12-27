#include <math.h>
#include <prstlib/adc.h>
#include <prstlib/button.h>
#include <prstlib/led.h>
#include <prstlib/macros.h>
#include <prstlib/sensors.h>
#include <prstlib/shtc3.h>
#include <ram_pwrdn.h>
#include <zb_nrf_platform.h>
#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zcl/zb_zcl_power_config.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>

#include "factory_reset.h"
#include "prst_zb_attrs.h"
#include "prst_zb_endpoint_defs.h"
#include "prst_zb_soil_moisture_defs.h"

LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

static struct zb_device_ctx dev_ctx;

static prst_sensors_t sensors;

static void led_flashing_cb(struct k_timer *timer) {
  prst_led_toggle();
}

K_TIMER_DEFINE(led_flashing_timer, led_flashing_cb, /*stop_fn=*/NULL);

ZB_ZCL_DECLARE_IDENTIFY_ATTRIB_LIST(
    identify_attr_list,
    &dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_EXT(
    basic_attr_list,
    &dev_ctx.basic_attr.zcl_version,
    &dev_ctx.basic_attr.app_version,
    &dev_ctx.basic_attr.stack_version,
    &dev_ctx.basic_attr.hw_version,
    &dev_ctx.basic_attr.mf_name,
    &dev_ctx.basic_attr.model_id,
    &dev_ctx.basic_attr.date_code,
    &dev_ctx.basic_attr.power_source,
    &dev_ctx.basic_attr.location_id,
    &dev_ctx.basic_attr.ph_env,
    &dev_ctx.basic_attr.sw_ver);

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
    /*battery_size=*/ZB_ZCL_POWER_CONFIG_BATTERY_SIZE_OTHER,
    /*battery_quantity=*/1,
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

ZB_ZCL_DECLARE_ILLUMINANCE_MEASUREMENT_ATTRIB_LIST(
    illuminance_attr_list,
    /*value=*/&dev_ctx.illuminance_attrs.log_lux,
    /*min_value=*/NULL,
    /*max_value*/ NULL);

PRST_ZB_DECLARE_CLUSTER_LIST(
    app_template_clusters,
    basic_attr_list,
    identify_attr_list,
    temp_measurement_attr_list,
    rel_humi_attr_list,
    batt_attr_list,
    soil_moisture_attr_list,
    illuminance_attr_list);

PRST_ZB_DECLARE_ENDPOINT(
    app_template_ep,
    PRST_ZIGBEE_ENDPOINT,
    app_template_clusters);

ZBOSS_DECLARE_DEVICE_CTX_1_EP(
    app_template_ctx,
    app_template_ep);

void zboss_signal_handler(zb_bufid_t bufid) {
  // See zigbee_default_signal_handler() for all available signals.
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, /*sg_p=*/NULL);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  switch (sig) {
    case ZB_BDB_SIGNAL_STEERING:         // New network.
    case ZB_BDB_SIGNAL_DEVICE_REBOOT: {  // Previously joined network.
      LOG_DBG("Steering complete. Status: %d", status);
      prst_led_flash(/*times=*/3);
      if (status == RET_OK) {
        k_timer_stop(&led_flashing_timer);
        prst_led_off();
      }
      break;
    }
    case ZB_ZDO_SIGNAL_SKIP_STARTUP:
    case ZB_ZDO_SIGNAL_LEAVE: {
      LOG_DBG("Will restart flashing");
      k_timer_start(&led_flashing_timer, K_NO_WAIT, K_SECONDS(1));
      break;
    }
  }

  ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
  if (bufid) {
    zb_buf_free(bufid);
  }
}

void update_sensors_cb(zb_uint8_t arg) {
  LOG_INF("Updating sensors");

  ZB_SCHEDULE_APP_ALARM(update_sensors_cb,
                        /*param=*/0,
                        ZB_TIME_ONE_SECOND * CONFIG_PRST_ZB_SLEEP_DURATION_SEC);

  if (prst_sensors_read_all(&sensors)) {
    LOG_ERR("Unable to read sensors");
    return;
  }

  // Battery voltage in units of 100 mV.
  uint8_t batt_voltage = sensors.batt.adc_read.millivolts / 100;
  prst_zb_set_attr_value(ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                         ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
                         &batt_voltage);

  // Battery percentage in units of 0.5%.
  zb_uint8_t batt_percentage = 2 * 100 * sensors.batt.percentage;
  prst_zb_set_attr_value(ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                         ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
                         &batt_percentage);

  // Temperature in units of 0.01 degrees Celcius.
  zb_int16_t temperature_value = 100 * sensors.shtc3.temp_c;
  prst_zb_set_attr_value(ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                         ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
                         &temperature_value);

  // Relative humidity in units of 0.01%.
  zb_int16_t rel_humi = 100 * 100 * sensors.shtc3.rel_humi;
  prst_zb_set_attr_value(ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
                         ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
                         &rel_humi);

  // Soil moisture in units of 0.01%.
  zb_int16_t soil_moisture = 100 * 100 * sensors.soil.percentage;
  prst_zb_set_attr_value(PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_CLUSTER_ID,
                         PRST_ZB_ZCL_ATTR_SOIL_MOISTURE_VALUE_ID,
                         &soil_moisture);

  // Illuminance in 10000 * log_10(lux) + 1.
  zb_int16_t log_lux = 10000 * log10((float)sensors.photo.brightness) + 1;
  prst_zb_set_attr_value(ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT,
                         ZB_ZCL_ATTR_ILLUMINANCE_MEASUREMENT_MEASURED_VALUE_ID,
                         &log_lux);
}

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());

  RET_IF_ERR(prst_zb_factory_reset_check());

  prst_zb_attrs_init(&dev_ctx);

  ZB_AF_REGISTER_DEVICE_CTX(&app_template_ctx);

  update_sensors_cb(/*arg=*/0);

  zb_zdo_pim_set_long_poll_interval(
      ZB_TIME_ONE_SECOND * CONFIG_PRST_ZB_PARENT_POLL_INTERVAL_SEC);
  power_down_unused_ram();

  RET_IF_ERR(prst_led_flash(2));
  k_msleep(100);

  zigbee_enable();
  zigbee_configure_sleepy_behavior(/*enable=*/true);

  return 0;
}
