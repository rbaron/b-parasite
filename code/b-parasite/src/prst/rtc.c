#include "prst/rtc.h"

#include <nrf_drv_rtc.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>

#include "prst_config.h"

// RTC0 is used by softdevice, so we need to pick another instance.
static const nrf_drv_rtc_t rtc_ = NRF_DRV_RTC_INSTANCE(2);
static prst_rtc_callback_t callback_handler_ = NULL;

static void rtc_callback(nrf_drv_rtc_int_type_t int_type) {
  if (int_type == NRF_DRV_RTC_INT_COMPARE2) {
    if (callback_handler_ != NULL) {
      callback_handler_();
    }
    // Reset RTC2 counter.
    nrf_drv_rtc_counter_clear(&rtc_);
    // We need to re-enable the RTC2 interrupt after rest.
    nrf_drv_rtc_int_enable(&rtc_, NRF_RTC_INT_COMPARE2_MASK);
  }
}

void prst_rtc_set_callback(prst_rtc_callback_t cb) { callback_handler_ = cb; }

void prst_rtc_init() {
  uint32_t err_code;
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  config.prescaler = 4095;
  err_code = nrf_drv_rtc_init(&rtc_, &config, rtc_callback);
  APP_ERROR_CHECK(err_code);

  // Disable events we're not interested in so they don't trigger interrupts.
  nrf_drv_rtc_tick_disable(&rtc_);
  nrf_drv_rtc_overflow_disable(&rtc_);

  // Make sure we're counting from 0.
  nrf_drv_rtc_counter_clear(&rtc_);

  // Set compare channel to trigger interrupt after specified time.
  err_code = nrf_drv_rtc_cc_set(&rtc_, 2, PRST_DEEP_SLEEP_IN_SECONDS * 8, true);
  APP_ERROR_CHECK(err_code);

  // Power on RTC instance.
  nrf_drv_rtc_enable(&rtc_);
}