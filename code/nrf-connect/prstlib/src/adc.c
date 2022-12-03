#include "prstlib/adc.h"

#include <drivers/adc.h>
#include <drivers/gpio.h>
#include <drivers/pwm.h>
#include <logging/log.h>
#include <math.h>
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

static const struct adc_dt_spec adc_batt_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))

static const struct adc_dt_spec adc_photo_transistor_spec =
    ADC_DT_SPEC_GET(DT_NODELABEL(photo_transistor));
struct gpio_dt_spec photo_transistor_enable_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(photo_transistor_enable), gpios);

#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))

static const struct adc_dt_spec adc_ldr_spec =
    ADC_DT_SPEC_GET(DT_NODELABEL(ldr));
struct gpio_dt_spec ldr_enable_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(ldr_enable), gpios);

#endif

static inline float get_soil_moisture_percent(float battery_voltage,
                                              int16_t raw_adc_output) {
  const double x = battery_voltage;
  const double dry = -11.7f * x * x + 101.0f * x + 306.0f;
  const double wet = 3.42f * x * x - 4.98f * x + 19.0f;
  const float percent = (raw_adc_output - dry) / (wet - dry);
  LOG_DBG("Read soil moisture: %.2f | Raw %u | Batt: %.2f | Dry: %.2f | Wet: %.2f",
          100.0f * percent, raw_adc_output, x, dry, wet);
  return percent;
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
  RET_IF_ERR(adc_channel_setup_dt(&adc_soil_spec));
  RET_IF_ERR(adc_channel_setup_dt(&adc_batt_spec));

  RET_IF_ERR(!device_is_ready(fast_disch_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&fast_disch_dt, GPIO_OUTPUT));

#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))
  RET_IF_ERR(adc_channel_setup_dt(&adc_photo_transistor_spec));
  RET_IF_ERR(!device_is_ready(photo_transistor_enable_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&photo_transistor_enable_dt, GPIO_OUTPUT));
#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))
  RET_IF_ERR(adc_channel_setup_dt(&adc_ldr_spec));
  RET_IF_ERR(!device_is_ready(ldr_enable_dt.port));
  RET_IF_ERR(gpio_pin_configure_dt(&ldr_enable_dt, GPIO_OUTPUT));
#endif

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
#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor))
  RET_IF_ERR(gpio_pin_set_dt(&photo_transistor_enable_dt, 1));
  k_msleep(10);
  RET_IF_ERR(read_adc_spec(&adc_photo_transistor_spec, &out->adc_read));
  RET_IF_ERR(gpio_pin_set_dt(&photo_transistor_enable_dt, 0));
  const float phototransistor_resistor = DT_PROP(DT_NODELABEL(photo_transistor), output_ohms);
  // Assuming 10000 lux for the saturation test. Calibration with a proper light
  // meter would be better.
  const float lux_sun = 10000.0f;
  const float current_sun = 3.59e-3f;
  const float current = out->adc_read.voltage / phototransistor_resistor;
  out->brightness = MAX(0, MIN(lux_sun * current / current_sun, UINT16_MAX));
  LOG_DBG("Read phototransistor: %u lx | %.2f V", out->brightness, out->adc_read.voltage);

#elif DT_NODE_EXISTS(DT_NODELABEL(ldr))
  RET_IF_ERR(gpio_pin_set_dt(&ldr_enable_dt, 1));
  k_msleep(10);
  RET_IF_ERR(read_adc_spec(&adc_ldr_spec, &out->adc_read));
  RET_IF_ERR(gpio_pin_set_dt(&ldr_enable_dt, 0));
  // The photo resistor forms a voltage divider with R.
  // The voltage here is measured in the middle of the voltage divider.
  // Vcc ---- (R_photo) ---|--- (R) ---- GND
  //                      Vout
  // So we can estimate R_photo = R * (Vcc - Vout) / Vout
  const float r = DT_PROP(DT_NODELABEL(ldr), output_ohms);
  const float photo_resistance =
      r * (battery_voltage - out->adc_read.voltage) / out->adc_read.voltage;
  // The relationship between the LDR resistance and the lux level is
  // logarithmic. We need to solve a logarithmic equation to find the lux
  // level, given the LDR resistance we just measured.
  // These values work for the GL5528 LDR and were borrowed from
  // https://github.com/QuentinCG/Arduino-Light-Dependent-Resistor-Library.
  const float mult_value = 32017200.0f;
  const float pow_value = 1.5832f;
  out->brightness =
      MAX(0, MIN(mult_value / powf(photo_resistance, pow_value), UINT16_MAX));
  LOG_DBG("Read LDR: %u lx | %.2f V", out->brightness, out->adc_read.voltage);

#endif

  return 0;
}