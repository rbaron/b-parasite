#include "prst/adc.h"

#include <app_error.h>
#include <nrf_drv_saadc.h>
#include <nrf_log.h>
#include <nrf_saadc.h>
// #include <nrfx_saadc.h>

#include "prst_config.h"

// 10 bits resoltuion.
#define PRST_ADC_RESOLUTION 10

// #define PRST_ADC_BATT_INPUT NRF_SAADC_INPUT_VDD
#define PRST_ADC_BATT_INPUT NRF_SAADC_INPUT_VDD
#define PRST_ADC_BATT_CHANNEL 0

#define PRST_ADC_SOIL_INPUT NRF_SAADC_INPUT_AIN1
#define PRST_ADC_SOIL_CHANNEL 1

static nrf_saadc_value_t sample_adc_channel(uint8_t channel) {
  nrf_saadc_value_t result;
  // *WARNING* this function is blocking, which is ot ideal but okay, but it
  // *does not work* when oversampling is set! I had to manually disable
  // SAADC_CONFIG_OVERSAMPLE in sdk_config.h.
  APP_ERROR_CHECK(nrf_drv_saadc_sample_convert(channel, &result));
  return result;
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
}

prst_adc_batt_read_t prst_adc_batt_read() {
  nrf_saadc_value_t result = sample_adc_channel(PRST_ADC_BATT_CHANNEL);
  prst_adc_batt_read_t ret;
  ret.raw = (uint16_t)result;
  ret.voltage = (3.6 * result) / (1 << PRST_ADC_RESOLUTION);
  ret.millivolts = ret.voltage * 1000;
#if PRST_ADC_DEBUG
  NRF_LOG_INFO("[adc] Read battery voltage: %d (raw); %d mV; ", ret.raw, ret.millivolts,
               ret.voltage);
#endif
  return ret;
}

int16_t prst_adc_soil_read() {
  nrf_saadc_value_t result = sample_adc_channel(PRST_ADC_SOIL_CHANNEL);
#if PRST_ADC_DEBUG
  NRF_LOG_INFO("[adc] Read soil moisture: %d", result);
#endif
  return result;
}