#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "prst/adc.h"
#include "prst/ble.h"
#include "prst/pwm.h"
#include "prst/rtc.h"
#include "prst/shtc3.h"
#include "prst_config.h"

// A small wrap-around counter for deduplicating BLE packets on the receiver.
static uint8_t run_counter = 0;

static void log_init(void) {
  APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Log inited.");
}

static void gpio_init(void) {
  nrf_gpio_cfg_output(PRST_LED_PIN);
  nrf_gpio_cfg_output(PRST_FAST_DISCH_PIN);
  NRF_LOG_INFO("GPIO pins inited.");
}

static void power_management_init(void) {
  APP_ERROR_CHECK(nrf_pwr_mgmt_init());
  NRF_LOG_INFO("GPIO pins inited.");
}

// This FPU exception mask trick is recommended for avoiding unwanted
// interupts from the floating point unit. This would be pretty bad,
// since it would wake us up from deep sleep for nothing.
#define FPU_EXCEPTION_MASK 0x0000009F
static void power_manage(void) {
  __set_FPSCR(__get_FPSCR() & ~(FPU_EXCEPTION_MASK));
  (void)__get_FPSCR();
  NVIC_ClearPendingIRQ(FPU_IRQn);
  nrf_pwr_mgmt_run();
}

// This is the RTC callback in which we do all of our work as quickly as
// possible:
// - Measure the soil moisture;
// - Measure the air temperature and humidity;
// - Encode the measurements into the BLE advertisement packet;
// - Turn on BLE advertising for a while;
// - Turn everything off and return back to sleep.
static void rtc_callback() {
  nrf_gpio_pin_set(PRST_LED_PIN);
  prst_shtc3_read_t temp_humi = prst_shtc3_read();
  nrf_gpio_pin_set(PRST_FAST_DISCH_PIN);
  prst_pwm_init();
  prst_pwm_start();
  prst_adc_batt_read_t batt_read = prst_adc_batt_read();
  prst_adc_soil_moisture_t soil_read = prst_adc_soil_read(batt_read.voltage);
  prst_pwm_stop();
  nrf_gpio_pin_clear(PRST_FAST_DISCH_PIN);

  nrf_gpio_pin_set(PRST_PHOTO_V);                                               // set GPIO for photoresistor HIGH
  prst_adc_photo_sensor_t photo_read = prst_adc_photo_read(batt_read.voltage);  // read PHOTO SENSOR value
  nrf_gpio_pin_clear(PRST_PHOTO_V);                                             // clear GPIO for photoresistor

  prst_ble_update_adv_data(batt_read.millivolts, temp_humi.temp_millicelcius,
                           temp_humi.humidity, soil_read.relative, photo_read.lux, run_counter);
  prst_adv_start();
  nrf_delay_ms(PRST_BLE_ADV_TIME_IN_MS);
  prst_adv_stop();
  nrf_gpio_pin_clear(PRST_LED_PIN);
  NRF_LOG_FLUSH();
  run_counter++;
}

int main(void) {
  log_init();
  gpio_init();
  power_management_init();
  prst_ble_init();
  prst_adc_init();
  prst_shtc3_init();

  // Set up RTC. It will call our custom callback at a regular interval, defined
  // by PRST_DEEP_SLEEP_IN_SECONDS.
  prst_rtc_set_callback(rtc_callback);
  prst_rtc_init();

  // In addition to scheduling it, let's immediatelly call it - it makes
  // debugging less tedious.
  rtc_callback();

  // Here we go into a low energy mode. The datasheet calls this mode "System
  // ON", and in my tests it consumes around 2.7uA.
  for (;;) {
    power_manage();
  }
}
