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
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>

#include "debug_counters.h"
#include "factory_reset.h"
#include "flash_fs.h"
#include "prst_zb_attrs.h"
#include "prst_zb_endpoint_defs.h"
#include "prst_zb_soil_moisture_defs.h"
#include "restart_handler.h"
#include "watchdog.h"

LOG_MODULE_REGISTER(app, CONFIG_LOG_DEFAULT_LEVEL);

static struct zb_device_ctx dev_ctx;

static prst_sensors_t sensors;

static bool joining_signal_received = false;
static bool stack_initialised = false;

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
    /*battery_size=*/&dev_ctx.batt_attrs.size,
    /*battery_quantity=*/&dev_ctx.batt_attrs.quantity,
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

void identify_cb(zb_bufid_t bufid) {
  LOG_DBG("Remote identify command called");
  prst_led_flash(15);
}

void update_sensors_cb(zb_uint8_t arg) {
  LOG_DBG("Updating sensors");

  // Reschedule the same callback.
  zb_ret_t ret = ZB_SCHEDULE_APP_ALARM(
      update_sensors_cb,
      /*param=*/0,
      ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000 * CONFIG_PRST_ZB_SLEEP_DURATION_SEC));
  if (ret != RET_OK) {
    prst_debug_counters_increment("sens_cb_schedule_err");
    __ASSERT(false, "Unable to schedule sensor update callback");
  }

  __ASSERT(!prst_watchdog_feed(), "Failed to feed watchdog");

  prst_debug_counters_increment("sensors_read_before");
  if (prst_sensors_read_all(&sensors)) {
    prst_debug_counters_increment("sensors_read_error");
    __ASSERT(false, "Unable to read sensors");
  }
  prst_debug_counters_increment("sensors_read_after");

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

void zboss_signal_handler(zb_bufid_t bufid) {
  // See zigbee_default_signal_handler() for all available signals.
  zb_zdo_app_signal_hdr_t *sig_hndler = NULL;
  zb_zdo_app_signal_type_t sig = zb_get_app_signal(bufid, /*sg_p=*/&sig_hndler);
  zb_ret_t status = ZB_GET_APP_SIGNAL_STATUS(bufid);
  switch (sig) {
    case ZB_BDB_SIGNAL_STEERING:         // New network.
    case ZB_BDB_SIGNAL_DEVICE_REBOOT: {  // Previously joined network.
      LOG_DBG("Steering complete. Status: %d", status);
      if (status == RET_OK) {
        LOG_DBG("Steering successful. Status: %d", status);
        k_timer_stop(&led_flashing_timer);
        prst_led_off();
        prst_restart_watchdog_stop();
        // Update the long polling parent interval - needs to be done after joining.
        zb_zdo_pim_set_long_poll_interval(1000 * CONFIG_PRST_ZB_PARENT_POLL_INTERVAL_SEC);
      } else {
        LOG_DBG("Steering failed. Status: %d", status);
        prst_restart_watchdog_start();
        // Power saving.
        k_timer_stop(&led_flashing_timer);
        prst_led_off();
      }
      joining_signal_received = true;
      break;
    }
    case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
      joining_signal_received = true;
      break;
    case ZB_ZDO_SIGNAL_LEAVE:
      if (status == RET_OK) {
        k_timer_start(&led_flashing_timer, K_NO_WAIT, K_SECONDS(1));
        zb_zdo_signal_leave_params_t *leave_params = ZB_ZDO_SIGNAL_GET_PARAMS(sig_hndler, zb_zdo_signal_leave_params_t);
        LOG_DBG("Network left (leave type: %d)", leave_params->leave_type);

        /* Set joining_signal_received to false so broken rejoin procedure can be detected correctly. */
        if (leave_params->leave_type == ZB_NWK_LEAVE_TYPE_REJOIN) {
          joining_signal_received = false;
        }
      }
      break;
    case ZB_ZDO_SIGNAL_SKIP_STARTUP: {
      stack_initialised = true;
      LOG_DBG("Started zigbee stack and waiting for connection to network.");
      k_timer_start(&led_flashing_timer, K_NO_WAIT, K_SECONDS(1));

      // Kick off main sensor update task.
      ZB_ERROR_CHECK(ZB_SCHEDULE_APP_ALARM(update_sensors_cb,
                                           /*param=*/0,
                                           ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000)));
      __ASSERT_NO_MSG(!prst_watchdog_start());
      break;
    }
    case ZB_NLME_STATUS_INDICATION: {
      zb_zdo_signal_nlme_status_indication_params_t *nlme_status_ind =
          ZB_ZDO_SIGNAL_GET_PARAMS(sig_hndler, zb_zdo_signal_nlme_status_indication_params_t);
      if (nlme_status_ind->nlme_status.status == ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE) {
        /* Check for broken rejoin procedure and restart the device to recover.
           This implements Nordic's suggested workaround for errata KRKNWK-12017, which effects
           the recent nRF Connect SDK (v1.8.0 - v2.3.0 at time of writing).
           For details see: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/known_issues.html?v=v2-3-0
         */
        if (stack_initialised && !joining_signal_received) {
          zb_reset(0);
        }
      }
      break;
    }
  }

  ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
  if (bufid) {
    zb_buf_free(bufid);
  }
}

void dump_counter(const char *counter_name, prst_debug_counter_t value) {
  LOG_INF("- %s: %d", counter_name, value);
}

int log_reset_reason_counter() {
  uint32_t cause;
  const char *reset_counter_str = "reset_cause_unknown";
  RET_IF_ERR(hwinfo_get_reset_cause(&cause));
  RET_IF_ERR(hwinfo_clear_reset_cause());
  if (cause & RESET_PIN) {
    reset_counter_str = "reset_cause_pin";
  } else if (cause & RESET_SOFTWARE) {
    // Includes fatal errors from __ASSERT, ZB_ERROR_CHECK and friends.
    reset_counter_str = "reset_cause_software";
  } else if (cause & RESET_BROWNOUT) {
    reset_counter_str = "reset_cause_brownout";
  } else if (cause & RESET_POR) {
    reset_counter_str = "reset_cause_power_on";
  } else if (cause & RESET_WATCHDOG) {
    reset_counter_str = "reset_cause_watchdog";
  } else if (cause & RESET_DEBUG) {
    reset_counter_str = "reset_cause_debug";
  } else if (cause & RESET_LOW_POWER_WAKE) {
    reset_counter_str = "reset_cause_low_power";
  } else if (cause & RESET_CPU_LOCKUP) {
    reset_counter_str = "reset_cause_cpu_lockup";
  } else if (cause & RESET_HARDWARE) {
    reset_counter_str = "reset_cause_hardware";
  } else if (cause & RESET_USER) {
    reset_counter_str = "reset_cause_user";
  }
  return prst_debug_counters_increment(reset_counter_str);
}

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());
  RET_IF_ERR(prst_flash_fs_init());
  RET_IF_ERR(prst_debug_counters_init());

  // Initialize sensors - quickly put them into low power mode.
  __ASSERT_NO_MSG(!prst_sensors_read_all(&sensors));

  prst_debug_counters_increment("boot");

  log_reset_reason_counter();

  LOG_INF("Dumping debug counters:");
  prst_debug_counters_get_all(dump_counter);

  __ASSERT_NO_MSG(!prst_zb_factory_reset_check());

  prst_zb_attrs_init(&dev_ctx);

  ZB_AF_REGISTER_DEVICE_CTX(&app_template_ctx);

  prst_led_flash(2);
  k_msleep(100);

  ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(PRST_ZIGBEE_ENDPOINT, identify_cb);

  zigbee_configure_sleepy_behavior(/*enable=*/true);
  power_down_unused_ram();
  zigbee_enable();

  prst_debug_counters_increment("main_finish");

  return 0;
}
