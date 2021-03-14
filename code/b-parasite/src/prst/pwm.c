#include "prst/pwm.h"

#include <app_error.h>
#include <nordic_common.h>
#include <nrf_drv_pwm.h>
#include <nrf_log.h>
#include <nrf_pwm.h>

#include "prst_config.h"

#define PRST_PWM_BASE_FREQ NRF_PWM_CLK_16MHz

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);

static nrf_pwm_values_common_t seq_values_[] = {8};
static const nrf_pwm_sequence_t seq_ = {
    .values.p_common = seq_values_,
    .length = NRF_PWM_VALUES_LENGTH(seq_values_),
    .repeats = 0,
    .end_delay = 0};

void prst_pwm_init() {
  UNUSED_VARIABLE(seq_);

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
      // .top_value = (uint16_t)(1e16 / PRST_PWM_FREQUENCY),
      .top_value = 16,
      .load_mode = NRF_PWM_LOAD_COMMON,
      .step_mode = NRF_PWM_STEP_AUTO};
  APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
}

void prst_pwm_start() {
  APP_ERROR_CHECK(
      nrf_drv_pwm_simple_playback(&m_pwm0, &seq_, 1, NRF_DRV_PWM_FLAG_LOOP));
}

void prst_pwm_stop() {}