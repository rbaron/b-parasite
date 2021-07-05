#include "prst/adc.h"

#include <app_error.h>
#include <nrf_drv_saadc.h>
#include <nrf_log.h>
#include <nrf_saadc.h>
#include <stdint.h>

#include "prst_config.h"

// 10 bits resoltuion.
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

  APP_ERROR_CHECK(
      nrf_drv_saadc_channel_init(PRST_ADC_PHOTO_CHANNEL, &photo_channel_config));
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

static inline double get_lux_level(double battery_voltage, nrf_saadc_value_t raw_adc_output) {
  const double x = battery_voltage;

  // todo: add DEBUG lines

  return 0;   // place holder. need to map values to lux level.
}

prst_adc_photo_sensor prst_adc_photo_read(double battery_voltage) {
  nrf_saadc_value_t raw_adc_output = sample_adc_channel(PRST_ADC_PHOTO_CHANNEL);
  const uint16_t lux = get_lux_level(battery_voltage, raw_adc_output);
  prst_adc_photo_sensor ret;
  ret.raw = raw_adc_output;
  ret.lux = lux;      

  // todo: add DEBUG lines
  
  return ret;
}