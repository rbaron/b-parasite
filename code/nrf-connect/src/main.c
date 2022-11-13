#include <drivers/adc.h>
#include <drivers/gpio.h>
#include <drivers/i2c.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// PWM_DT_SPEC_GET(DT_LABEL(pwm0));
// static const struct pwm_dt_spec soil_pwm_dt =
//     PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
// static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

// static const struct adc_channel_cfg soil_adc_config =
//     // ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc), channel_0));
//     ADC_CHANNEL_CFG_DT(DT_NODELABEL(soil_adc_channel));
// static const struct adc_dt_spec soil_adc_spec =
// ADC_CHANNEL_CFG_DT(DT_CHILD(DT_NODELABEL(adc), channel_0));
// ADC_DT_SPEC_GET(DT_NODELABEL(soil_adc_channel));
static const struct adc_dt_spec adc_soil_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

// #define ERR_CHECK(expr) \
//   {                     \
//     err = expr; \
// if (err != 0) {\
// LOG_ERR("Error " __lineno__);
// }\
// }

void main(void) {
  // prst_shtc3_read_t shtc3_read = prst_shtc3_read();
  // pwm_set_pulse_dt(&soil_pwm_dt, pulse);
  int err;
  int16_t buf;
  int32_t val_mv;
  struct adc_sequence sequence = {
      .buffer = &buf,
      /* buffer size in bytes, not number of samples */
      .buffer_size = sizeof(buf),
  };

  err = adc_channel_setup_dt(&adc_soil_spec);
  if (err) {
    LOG_ERR("Error in adc_channel_setup_dt");
  }

  err = adc_sequence_init_dt(&adc_soil_spec, &sequence);
  if (err) {
    LOG_ERR("Error in adc_sequence_init_dt");
  }
  while (true) {
    err = adc_read(adc_soil_spec.dev, &sequence);
    if (err) {
      LOG_ERR("Error in adc_read");
    }
    val_mv = buf;
    err = adc_raw_to_millivolts_dt(&adc_soil_spec, &val_mv);
    if (err < 0) {
      printk(" (value in mV not available)\n");
    } else {
      printk(" = %u mV\n", val_mv);
    }
    k_msleep(500);
  }
}
