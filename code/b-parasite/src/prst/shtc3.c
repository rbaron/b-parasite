#include "prst/shtc3.h"

#include <app_error.h>
#include <nrf_delay.h>
#include <nrf_drv_twi.h>
#include <nrf_gpio.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>

// #define PRST_SHT3C_DEFAULT_ADDR 0x70
// I'm using a sht30 to test while I wait for the new PCBs.
#define PRST_SHT3C_DEFAULT_ADDR 0x44

static const nrf_drv_twi_t twi_ = NRF_DRV_TWI_INSTANCE(0);
static nrf_drv_twi_config_t twi_config_ = NRF_DRV_TWI_DEFAULT_CONFIG;

static uint8_t buff[6];

void prst_shtc3_init() {
  twi_config_.scl = NRF_GPIO_PIN_MAP(0, 3);
  twi_config_.sda = NRF_GPIO_PIN_MAP(0, 2);
  // twi_config.clear_bus_init = true;
  twi_config_.frequency = NRF_TWI_FREQ_100K;
}

prst_shtc3_read_t prst_shtc3_read() {
  uint32_t err_code;
  // Sends request for data with clock stretching. Currently not working
  // very well - sometimes it works, sometimes the read blocks forever.
  // uint8_t tx_data[] = {0x2c, 0x06};
  // Request for data withour clock stretching. If no data is available yet,
  // the result will be a NACK.
  uint8_t tx_data[] = {0x24, 0x00};

  err_code = nrf_drv_twi_init(&twi_, &twi_config_, NULL, NULL);

  APP_ERROR_CHECK(err_code);
  nrf_delay_ms(10);
  nrf_drv_twi_enable(&twi_);
  nrf_delay_ms(10);
  err_code = nrf_drv_twi_tx(&twi_, PRST_SHT3C_DEFAULT_ADDR, tx_data,
                            sizeof(tx_data), false);
  APP_ERROR_CHECK(err_code);

  // TODO(rbaron): timeout.
  while (true) {
    err_code = nrf_drv_twi_rx(&twi_, PRST_SHT3C_DEFAULT_ADDR, buff, 6);
    if (err_code == NRF_ERROR_DRV_TWI_ERR_ANACK) {
      nrf_delay_ms(10);
      continue;
    }
    break;
  }

  // TODO(rbaron): put the sensor to sleep & save power.

  nrf_drv_twi_uninit(&twi_);
  NRF_LOG_INFO("Computing...");
  double temp_c =
      -45 + 175 * ((double)((buff[0] << 8) | buff[1])) / ((1 << 16) - 1);
  // double humi = 100 * ( (double) ((buff[0] << 8) | buff[1])) / ((1<<16) - 1);
  prst_shtc3_read_t ret = {.temp_c = temp_c, .humidity = 0};
  return ret;
}