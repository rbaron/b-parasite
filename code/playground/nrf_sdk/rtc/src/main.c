#include <stdbool.h>
#include <stdint.h>

#include "app_error.h"
#include "boards.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_rtc.h"
#include "nrf_gpio.h"

// P0.03
#define LED_PIN 3

// Seconds between RTC COMPARE0 events.
#define COMPARE_COUNTERTIME (3UL)

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0);

static void rtc_handler(nrf_drv_rtc_int_type_t int_type) {
  if (int_type == NRF_DRV_RTC_INT_COMPARE0) {
    nrf_gpio_pin_toggle(LED_PIN);
    nrf_drv_rtc_counter_clear(&rtc);
    // We need to re-enable the COMPARE0 interrupt.
    nrf_drv_rtc_int_enable(&rtc, NRF_RTC_INT_COMPARE0_MASK);
  }
  // This should be disabled and never triggered.
  else if (int_type == NRF_DRV_RTC_INT_TICK) {
  }
}

static void lfclk_config(void) {
  ret_code_t err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);

  nrf_drv_clock_lfclk_request(NULL);
}

static void rtc_config(void) {
  uint32_t err_code;

  // Initialize RTC instance.
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  config.prescaler = 4095;
  err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
  APP_ERROR_CHECK(err_code);

  nrf_drv_rtc_tick_disable(&rtc);
  nrf_drv_rtc_overflow_disable(&rtc);
  nrf_drv_rtc_counter_clear(&rtc);

  // Set compare channel to trigger interrupt after COMPARE_COUNTERTIME
  // seconds.
  err_code = nrf_drv_rtc_cc_set(&rtc, 0, COMPARE_COUNTERTIME * 8, true);
  APP_ERROR_CHECK(err_code);

  // Power on RTC instance.
  nrf_drv_rtc_enable(&rtc);
}

int main(void) {
  nrf_gpio_cfg_output(LED_PIN);

  lfclk_config();

  rtc_config();

  while (true) {
    // NRF_POWER->SYSTEMOFF = 1;
    __SEV();
    __WFE();
    __WFE();
  }
}