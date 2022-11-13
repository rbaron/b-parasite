#include "adc.h"

#include <drivers/adc.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_REGISTER(adc, LOG_LEVEL_DBG);

// PWM spec for square wave. Input to the soil sensing circuit.
static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

// Shared buffer and adc_sequennce.
static int16_t buf;
static struct adc_sequence sequence = {
    .buffer = &buf,
    .buffer_size = sizeof(buf),
};

static const struct adc_dt_spec adc_soil_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

static const struct adc_dt_spec adc_lux_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

static const struct adc_dt_spec adc_batt_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 2);

// TODO: recalibrate with v2 hardware.
static inline float get_soil_moisture_percent(float battery_voltage,
                                              int16_t raw_adc_output) {
  const double x = battery_voltage;
  const double dry = -12.9 * x * x + 111 * x + 228;
  const double wet = -5.71 * x * x + 60.2 * x + 126;
  LOG_DBG("Batt: %.2f", x);
  LOG_DBG("Dry: %.2f | wet: %.2f", dry, wet);
  return (raw_adc_output - dry) / (wet - dry);
}

static int read_adc_spec(const struct adc_dt_spec* spec, prst_adc_read_t* out) {
  int err;
  err = adc_sequence_init_dt(spec, &sequence);
  if (err) {
    LOG_ERR("Error in adc_sequence_init_dt");
    return err;
  }
  err = adc_read(spec->dev, &sequence);
  if (err) {
    LOG_ERR("Error in adc_read");
    return err;
  }

  int32_t val_mv = buf;
  err = adc_raw_to_millivolts_dt(spec, &val_mv);
  if (err) {
    LOG_ERR("Error in adc_read");
    return err;
  }

  out->raw = buf;
  out->millivolts = val_mv;
  out->voltage = val_mv / 1000.0f;
  return 0;
}

int prst_adc_init() {
  const struct adc_dt_spec* all_specs[] = {
      &adc_soil_spec,
      &adc_lux_spec,
      &adc_batt_spec,
  };
  int err;
  for (int i = 0; i < sizeof(all_specs) / sizeof(all_specs[0]); i++) {
    err = adc_channel_setup_dt(all_specs[i]);
    if (err) {
      LOG_ERR("Error setting up adc_soil_spec");
      return err;
    }
  }
  return 0;
}

prst_adc_read_t prst_adc_batt_read() {
  int err;
  prst_adc_read_t adc_read;
  err = read_adc_spec(&adc_batt_spec, &adc_read);
  if (err) {
    LOG_ERR("Error in prst_adc_batt_read");
  }
  return adc_read;
}

prst_adc_soil_moisture_t prst_adc_soil_read(float battery_voltage) {
  int err;
  // Start PWM.
  err = pwm_set_pulse_dt(&soil_pwm_dt, pulse);
  if (err) {
    LOG_ERR("Error in pwm_set_pulse_dt");
  }
  k_msleep(30);

  prst_adc_soil_moisture_t adc_soil_read;
  err = read_adc_spec(&adc_soil_spec, &adc_soil_read.adc_read);
  if (err) {
    LOG_ERR("Error in prst_adc_batt_read");
  }

  // Stop PWM.
  err = pwm_set_pulse_dt(&soil_pwm_dt, 0);
  if (err) {
    LOG_ERR("Error in pwm_set_pulse_dt");
  }

  adc_soil_read.percentage = get_soil_moisture_percent(battery_voltage, buf);
  return adc_soil_read;
}

prst_adc_photo_sensor_t prst_adc_photo_read(float battery_voltage) {
  // int err;
  // // Start PWM.
  // err = pwm_set_pulse_dt(&soil_pwm_dt, pulse);
  // if (err) {
  //   LOG_ERR("Error in pwm_set_pulse_dt");
  // }
  // k_msleep(30);

  // err = adc_sequence_init_dt(&adc_soil_spec, &sequence);
  // if (err) {
  //   LOG_ERR("Error in adc_sequence_init_dt");
  // }
  // err = adc_read(adc_soil_spec.dev, &sequence);
  // if (err) {
  //   LOG_ERR("Error in adc_read");
  // }
  prst_adc_photo_sensor_t ret;
  return ret;
}
