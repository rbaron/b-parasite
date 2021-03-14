#include <stdbool.h>
#include <stdint.h>

#include "app_timer.h"
#include "ble_advdata.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_drv_rtc.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_soc.h"
#include "prst/ble.h"
#include "prst/rtc.h"

// P0.03
// #define LED_PIN 3
#define LED_PIN NRF_GPIO_PIN_MAP(1, 11)

// Environmental sensing.
#define SERVICE_UUID 0x181a

#define DEAD_BEEF 0xDEADBEEF

void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Log inited");
}

static void leds_init(void) {
  nrf_gpio_cfg_output(LED_PIN);
  nrf_gpio_pin_toggle(LED_PIN);
  nrf_delay_ms(500);
  nrf_gpio_pin_toggle(LED_PIN);
  nrf_delay_ms(500);
  NRF_LOG_INFO("Leds inited");
}

static void timers_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

#define FPU_EXCEPTION_MASK 0x0000009F

static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

//
static void power_manage(void) {
  __set_FPSCR(__get_FPSCR() & ~(FPU_EXCEPTION_MASK));
  (void)__get_FPSCR();
  NVIC_ClearPendingIRQ(FPU_IRQn);
  nrf_pwr_mgmt_run();
}

static void rtc_callback() {
  NRF_LOG_INFO("rtc callback running...\n");
  NRF_LOG_FLUSH();
  nrf_gpio_pin_set(LED_PIN);
  prst_adv_start();
  nrf_delay_ms(300);
  prst_adv_stop();
  nrf_gpio_pin_clear(LED_PIN);
}

int main(void) {
  // Initialize.
  log_init();
  leds_init();
  power_management_init();
  prst_ble_init();
  prst_ble_update_adv_data(1);

  prst_rtc_set_callback(rtc_callback);
  prst_rtc_init();

  // Enter main loop.
  for (;;) {
    power_manage();
  }
}