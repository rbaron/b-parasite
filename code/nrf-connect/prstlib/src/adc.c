#include "prstlib/adc.h"

#include <math.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "prstlib/macros.h"

LOG_MODULE_REGISTER(adc, CONFIG_PRSTLIB_LOG_LEVEL);

// PWM spec for square wave. Input to the soil sensing circuit.
static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

// Calibration coefficients for the soil sensing circuit.
static const int dry_coeffs[3] = DT_PROP(DT_NODELABEL(soil_calibration_coeffs), dry);
static const int wet_coeffs[3] = DT_PROP(DT_NODELABEL(soil_calibration_coeffs), wet);

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

typedef struct {
  // High (h) and low (p) voltage (v) and % (p) points.
  float vh, vl, ph, pl;
} batt_disch_linear_section_t;

static void set_battery_percent(const prst_adc_read_t* read, prst_batt_t* out) {
  // Must be sorted by .vh.
  static const batt_disch_linear_section_t sections[] = {
      {.vh = 3.00f, .vl = 2.90f, .ph = 1.00f, .pl = 0.42f},
      {.vh = 2.90f, .vl = 2.74f, .ph = 0.42f, .pl = 0.18f},
      {.vh = 2.74f, .vl = 2.44f, .ph = 0.18f, .pl = 0.06f},
      {.vh = 2.44f, .vl = 2.01f, .ph = 0.06f, .pl = 0.00f},
  };

  const float v = read->voltage;

  if (v > sections[0].vh) {
    out->percentage = 1.0f;
    return;
  }
  for (int i = 0; i < ARRAY_SIZE(sections); i++) {
    const batt_disch_linear_section_t* s = &sections[i];
    if (v > s->vl) {
      out->percentage = s->pl + (v - s->vl) * ((s->ph - s->pl) / (s->vh - s->vl));
      return;
    }
  }
  out->percentage = 0.0f;
  return;
}

static inline float eval_poly(const int coeffs[3], float x) {
  // The coefficients are specified times 1000, as a workaround the lack of support for floating
  // points in devicetree bindings.
  return (coeffs[0] + coeffs[1] * x + coeffs[2] * x * x) / 1000.0f;
}

static inline float get_soil_moisture_percent(float battery_voltage,
                                              int16_t raw_adc_output) {
  const float x = battery_voltage;
  const float dry = eval_poly(dry_coeffs, x);
  const float wet = eval_poly(wet_coeffs, x);
  const float percent = (raw_adc_output - dry) / (wet - dry);
  LOG_DBG("Read soil moisture 2: %.2f | Raw %u | Batt: %.2f | Dry: %.2f | Wet: %.2f",
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

  for (size_t idx = 0; idx < ARRAY_SIZE(dry_coeffs); idx++) {
    LOG_DBG("Dry coeff %d: %d\n", idx, dry_coeffs[idx]);
  }
  for (size_t idx = 0; idx < ARRAY_SIZE(wet_coeffs); idx++) {
    LOG_DBG("Wet coeff %d: %d\n", idx, wet_coeffs[idx]);
  }
  return 0;
}

int prst_adc_batt_read(prst_batt_t* out) {
  RET_IF_ERR(read_adc_spec(&adc_batt_spec, &out->adc_read));
  set_battery_percent(&out->adc_read, out);
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
