#include <drivers/i2c.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// PWM_DT_SPEC_GET(DT_LABEL(pwm0));
static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

void main(void) {
  // prst_shtc3_read_t shtc3_read = prst_shtc3_read();
  pwm_set_pulse_dt(&soil_pwm_dt, pulse);

  while (true) {
    k_msleep(500);
  }
}
