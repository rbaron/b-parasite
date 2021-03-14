#include "prst/pwm.h"

#include <app_error.h>
#include <nordic_common.h>
#include <nrf_drv_pwm.h>
#include <nrf_log.h>
#include <nrf_pwm.h>

#include "prst_config.h"

// Each step in the counter will take 1/16e6 s.
#define PRST_PWM_BASE_FREQ NRF_PWM_CLK_16MHz
// We will count up to 16. It will take 1us at 16MHz.
// With the NRF_PWM_MODE_UP_AND_DOWN count mode, we assume 1us is half the
// output PWM period (total 2us => 500MHz frequency). We set a duty cycle of
// 50% below with PRST_PWM_FLIP_AT_COUNT to be half the max count.
#define PRST_PWM_MAX_COUNT 16
// We will toggle the PWM output when we reach this count.
#define PRST_PWM_FLIP_AT_COUNT PRST_PWM_MAX_COUNT / 2

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static nrf_pwm_values_common_t seq_values_[] = {PRST_PWM_FLIP_AT_COUNT};
static const nrf_pwm_sequence_t seq_ = {
    .values.p_common = seq_values_,
    .length = NRF_PWM_VALUES_LENGTH(seq_values_),
    .repeats = 0,
    .end_delay = 0};

void prst_pwm_init() {
  nrf_drv_pwm_config_t const config0 = {
      .output_pins =
          {
              PRST_PWM_PIN | NRF_DRV_PWM_PIN_INVERTED,  // channel 0
              NRF_DRV_PWM_PIN_NOT_USED,                 // channel 1
              NRF_DRV_PWM_PIN_NOT_USED,                 // channel 2
              NRF_DRV_PWM_PIN_NOT_USED,                 // channel 3
          },
      .irq_priority = APP_IRQ_PRIORITY_LOWEST,
      // This is the hal PRESCALER.
      .base_clock = NRF_PWM_CLK_16MHz,
      // This is the hal COUNTERTOP.
      .count_mode = NRF_PWM_MODE_UP_AND_DOWN,
      .top_value = PRST_PWM_MAX_COUNT,
      .load_mode = NRF_PWM_LOAD_COMMON,
      .step_mode = NRF_PWM_STEP_AUTO};
  APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
}

void prst_pwm_start() {
  // Loop forever.
  APP_ERROR_CHECK(
      nrf_drv_pwm_simple_playback(&m_pwm0, &seq_, 1, NRF_DRV_PWM_FLAG_LOOP));
}

void prst_pwm_stop() {
  APP_ERROR_CHECK(nrf_drv_pwm_stop(&m_pwm0, /*wait_until_stopped=*/true));
  nrf_drv_pwm_uninit(&m_pwm0);
}