#include "prstlib/adc.h"

#include <drivers/adc.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prstlib/macros.h"

LOG_MODULE_REGISTER(adc, LOG_LEVEL_WRN);

// PWM spec for square wave. Input to the soil sensing circuit.
static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

struct gpio_dt_spec fast_disch_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(fast_disch), gpios);

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
  const double dry = -11.7f * x * x + 101.0f * x + 306.0f;
  const double wet = 3.42f * x * x - 4.98f * x + 19.0f;
  LOG_DBG("Raw %u | Batt: %.2f | Dry: %.2f | Wet: %.2f", raw_adc_output, x, dry,
          wet);
  return (raw_adc_output - dry) / (wet - dry);
}

static int read_adc_spec(const struct adc_dt_spec* spec, prst_adc_read_t* out) {
  RET_IF_ERR(adc_sequence_init_dt(spec, &sequence));

  RET_IF_ERR(adc_read(spec->dev, &sequence));

  int32_t val_mv = buf;
  RET_IF_ERR(adc_raw_to_millivolts_dt(spec, &val_mv));

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
  for (int i = 0; i < sizeof(all_specs) / sizeof(all_specs[0]); i++) {
    RET_IF_ERR(adc_channel_setup_dt(all_specs[i]));
  }

  RET_IF_ERR(!device_is_ready(fast_disch_dt.port));
  return gpio_pin_configure_dt(&fast_disch_dt, GPIO_OUTPUT);
  return 0;
}

int prst_adc_batt_read(prst_adc_read_t* out) {
  RET_IF_ERR(read_adc_spec(&adc_batt_spec, out));
  return 0;
}

int prst_adc_soil_read(float battery_voltage, prst_adc_soil_moisture_t* out) {
  // Enable fast discharge circuit.
  RET_IF_ERR(gpio_pin_set_dt(&fast_disch_dt, 1));

  // Start PWM.
  RET_IF_ERR(pwm_set_dt(&soil_pwm_dt, soil_pwm_dt.period, pulse));

  k_msleep(30);

  RET_IF_ERR(read_adc_spec(&adc_soil_spec, &out->adc_read));

  // Stop PWM.
  RET_IF_ERR(pwm_set_dt(&soil_pwm_dt, 0, 0));

  // Turn off fast discharge circuit.
  RET_IF_ERR(gpio_pin_set_dt(&fast_disch_dt, 0));

  out->percentage =
      MAX(0.0f, MIN(1.0f, get_soil_moisture_percent(battery_voltage, buf)));
  return 0;
}

int prst_adc_photo_read(float battery_voltage, prst_adc_photo_sensor_t* out) {
  RET_IF_ERR(read_adc_spec(&adc_soil_spec, &out->adc_read));

  // The ALS-PT19 phototransistor is a device in which the current flow between
  // its two terminals is controlled by how much light there is in the ambient.
  // We measure that current by calculating the voltage across a resistor that
  // is connected in series with the phototransistor.
  const float phototransistor_resistor = 470.0f;
  const float current_sun = 3.59e-3f;
  // Assuming 10000 lux for the saturation test. Calibration with a proper light
  // meter would be better.
  const float lux_sun = 10000.0f;
  const float current = out->adc_read.voltage / phototransistor_resistor;
  out->brightness = MAX(0, MIN(lux_sun * current / current_sun, UINT16_MAX));
  return 0;
}
