#include "prst/adc.h"

#include <app_error.h>
#include <math.h>
#include <nrf_drv_saadc.h>
#include <nrf_log.h>
#include <nrf_saadc.h>
#include <stdint.h>

#include "prst_config.h"

#define PRST_ADC_RESOLUTION 10

#define PRST_ADC_BATT_INPUT NRF_SAADC_INPUT_VDD
#define PRST_ADC_BATT_CHANNEL 0

#define PRST_ADC_SOIL_INPUT NRF_SAADC_INPUT_AIN1
#define PRST_ADC_SOIL_CHANNEL 1

#define PRST_ADC_PHOTO_INPUT NRF_SAADC_INPUT_AIN0
#define PRST_ADC_PHOTO_CHANNEL 2

static nrf_saadc_value_t sample_adc_channel(uint8_t channel) {
  nrf_saadc_value_t result;
  // *WARNING* this function is blocking, which is ot ideal but okay, but it
  // *does not work* when oversampling is set! I had to manually disable
  // SAADC_CONFIG_OVERSAMPLE in sdk_config.h.
  APP_ERROR_CHECK(nrf_drv_saadc_sample_convert(channel, &result));
  return result;
}

// Caps the argument to the [0.0, 1.0] range.
static inline double cap_percentage(double value) {
  return value > 1.0 ? 1.0 : (value < 0.0 ? 0.0 : value);
}

// Unused, since we'll call the SAADC synchronously for now.
void saadc_callback(nrf_drv_saadc_evt_t const* p_event) {
  if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
    ret_code_t err_code;
    uint16_t size = p_event->data.done.size;

    err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, size);
    APP_ERROR_CHECK(err_code);

    int i;
    NRF_LOG_INFO("[adc] ADC event!");

    for (i = 0; i < size; i++) {
      NRF_LOG_INFO("[adc] %d", p_event->data.done.p_buffer[i]);
    }
  }
}

void prst_adc_init() {
  nrf_saadc_channel_config_t batt_channel_config =
      NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PRST_ADC_BATT_INPUT);

  APP_ERROR_CHECK(nrf_drv_saadc_init(NULL, saadc_callback));

  APP_ERROR_CHECK(
      nrf_drv_saadc_channel_init(PRST_ADC_BATT_CHANNEL, &batt_channel_config));

  nrf_saadc_channel_config_t soil_channel_config =
      NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PRST_ADC_SOIL_INPUT);
  soil_channel_config.reference = NRF_SAADC_REFERENCE_VDD4;
  APP_ERROR_CHECK(
      nrf_drv_saadc_channel_init(PRST_ADC_SOIL_CHANNEL, &soil_channel_config));

  nrf_saadc_channel_config_t photo_channel_config =
      NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PRST_ADC_PHOTO_INPUT);
  APP_ERROR_CHECK(nrf_drv_saadc_channel_init(PRST_ADC_PHOTO_CHANNEL,
                                             &photo_channel_config));
}

prst_adc_batt_read_t prst_adc_batt_read() {
  nrf_saadc_value_t result = sample_adc_channel(PRST_ADC_BATT_CHANNEL);
  prst_adc_batt_read_t ret;
  ret.raw = (uint16_t)result;
  ret.voltage = (3.6 * result) / (1 << PRST_ADC_RESOLUTION);
  ret.millivolts = ret.voltage * 1000;
#if PRST_ADC_BATT_DEBUG
  NRF_LOG_INFO(
      "[adc] Read battery voltage: %d (raw); %d mV; " NRF_LOG_FLOAT_MARKER " V",
      ret.raw, ret.millivolts, NRF_LOG_FLOAT(ret.voltage));
#endif
  return ret;
}

// If you got this far and really want to see how the sausage is made,
// this function estimates the soil moisture percent based on the raw
// ADC value as returned from the saadc. It assumes 10 bits resolution.
// Ideally, we're taking the ADC sample relative to the VDD voltage, so
// this input value should be stable across the range of input voltages.
// In practice, when varying the input voltage, this value is drifting
// enough to be annoying. To account for this drift, I collected ADC readings
// while varying the input voltage from 2V to 3V (CR2032 voltage range) and
// fitted two second degree polynomials over them - one for the sensor
// out in the air (representing a dry soil) and one while holding the
// sensor in my hand (representing a wet soil).
// This raw data is available at the data/ dir at the root of this repository.
static inline double get_soil_moisture_percent(
    double battery_voltage, nrf_saadc_value_t raw_adc_output) {
  const double x = battery_voltage;
  const double dry = -12.9 * x * x + 111 * x + 228;
  const double wet = -5.71 * x * x + 60.2 * x + 126;
#if PRST_ADC_SOIL_DEBUG
  NRF_LOG_INFO("[adc] batt: " NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(x));
  NRF_LOG_INFO("[adc] dry: " NRF_LOG_FLOAT_MARKER " wet: " NRF_LOG_FLOAT_MARKER,
               NRF_LOG_FLOAT(dry), NRF_LOG_FLOAT(wet));
#endif
  return (raw_adc_output - dry) / (wet - dry);
}

prst_adc_soil_moisture_t prst_adc_soil_read(double battery_voltage) {
  nrf_saadc_value_t raw_adc_output = sample_adc_channel(PRST_ADC_SOIL_CHANNEL);
  const double percentage =
      get_soil_moisture_percent(battery_voltage, raw_adc_output);
  prst_adc_soil_moisture_t ret;
  ret.raw = raw_adc_output;
  ret.percentage = percentage;
  ret.relative = cap_percentage(percentage) * UINT16_MAX;
#if PRST_ADC_SOIL_DEBUG
  NRF_LOG_INFO("[adc] Read soil moisture: %d (raw); " NRF_LOG_FLOAT_MARKER
               " %% (percentage); %u (relative)",
               ret.raw, NRF_LOG_FLOAT(percentage * 100), ret.relative);
#endif
  return ret;
}

prst_adc_photo_sensor_t prst_adc_photo_read(double battery_voltage) {
  nrf_saadc_value_t raw_photo_output =
      sample_adc_channel(PRST_ADC_PHOTO_CHANNEL);

  if (raw_photo_output < 0) {
    raw_photo_output = 0;
  }

  prst_adc_photo_sensor_t ret;
  ret.raw = raw_photo_output;
  ret.voltage = (3.6 * raw_photo_output) / (1 << PRST_ADC_RESOLUTION);
  // ret.voltage = (2.4 * raw_photo_output) / (1 << PRST_ADC_RESOLUTION);

#if PRST_HAS_LDR
  // The photo resistor forms a voltage divider with a 10 kOhm resistor.
  // The voltage here is measured in the middle of the voltage divider.
  // Vcc ---- (R_photo) ---|--- (10k) ---- GND
  //                      Vout
  // So we can estimate R_photo = R * (Vcc - Vout) / Vout
  const float photo_resistance =
      1e4f * (battery_voltage - ret.voltage) / ret.voltage;

  // The relationship between the LDR resistance and the lux level is
  // logarithmic. We need to solve a logarithmic equation to find the lux
  // level, given the LDR resistance we just measured.
  // These values work for the GL5528 LDR and were borrowed from
  // https://github.com/QuentinCG/Arduino-Light-Dependent-Resistor-Library.
  const float mult_value = 32017200.0f;
  const float pow_value = 1.5832f;
  ret.brightness =
      MAX(0, MIN(mult_value / powf(photo_resistance, pow_value), UINT16_MAX));

#elif PRST_HAS_PHOTOTRANSISTOR
  // The ALS-PT19 phototransistor is a device in which the current flow between
  // its two terminals is controlled by how much light there is in the ambient.
  // We measure that current by calculating the voltage across a resistor that
  // is connected in series with the phototransistor.
  // Some infor:
  // - Not all lights are the same. The ALS-PT19 has different current
  // responses for incandescent and fluorescent lights and it shows no values
  // for sunlight. We have to make some compromises here, aiming for a
  // "reasonable" middle ground through calibration.
  // - ALS-PT19' minimum voltage is 2.5 V. Our CR2032 battery has a theoretical
  // range of 2.0 V to 3.0 V, but in our usage pattern it seems to spend most of
  // its life around 2.6 V (https://github.com/rbaron/b-parasite/issues/1). In
  // my manual tests it seems we can usually go lower than that. So while we're
  // pushing it a bit, we should be okay most of the time.
  //
  // In order to design the value of the series resistor, we assume Vcc = 2.5V,
  // and we want the upper limit of light intensity to correspond to a value of
  // Vout < 2.5 V - 0.4 V (saturation) = 2.1 V. Let's call this the "direct
  // sunlight" voltage across our resistor. This direct sunlight voltage will
  // appear when the phototransistor outputs the direct sunlight current.
  //
  // In short, what we want:
  // A value of R_L such that Vout is < 2.1 V (but close) when the sensor is in
  // direct sunlight. While Vcc is 2.5 V, R_L = 470 Ohm outputs Vout ~ 1.7V, so
  // there's still some wiggle room for even more sunnier days.
  //
  // Another caveat: the datasheet shows that the current in the transistor is
  // relatively constant when we vary Vcc (Fig.4). This seems to be true for low
  // currents (the datsheet uses 100 lx in Fig.4), but not for larger currents.
  const float phototransistor_resistor = 470.0f;
  const float current_sun = 3.59e-3f;
  // Assuming 10000 lux for the saturation test. Calibration with a proper light
  // meter would be better.
  const float lux_sun = 10000.0f;

  const float current = ret.voltage / phototransistor_resistor;
  ret.brightness = MAX(0, MIN(lux_sun * current / current_sun, UINT16_MAX));

#if PRST_ADC_PHOTO_DEBUG
  NRF_LOG_INFO("[adc] Phototransistor current: " NRF_LOG_FLOAT_MARKER " uA",
               NRF_LOG_FLOAT(1000000 * current));
#endif  // PRST_ADC_PHOTO_DEBUG
#endif  // PRST_HAS_PHOTOTRANSISTOR

#if PRST_ADC_PHOTO_DEBUG
  NRF_LOG_INFO("[adc] Read brightness level: " NRF_LOG_FLOAT_MARKER
               " mV %d (raw); %d (lux)",
               NRF_LOG_FLOAT(1000 * ret.voltage), ret.raw, ret.brightness);
#endif
  return ret;
}
