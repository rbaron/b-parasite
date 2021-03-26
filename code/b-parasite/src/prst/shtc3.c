#include "prst/shtc3.h"

#include <app_error.h>
#include <nrf_delay.h>
#include <nrf_drv_twi.h>
#include <nrf_log.h>
#include <nrf_log_ctrl.h>

static const nrf_drv_twi_t twi_ = NRF_DRV_TWI_INSTANCE(0);
static nrf_drv_twi_config_t twi_config_ = NRF_DRV_TWI_DEFAULT_CONFIG;

static uint8_t buff[6];

static void write_cmd(uint16_t command) {
  uint8_t cmd[2];
  cmd[0] = command >> 8;
  cmd[1] = command & 0xff;
  APP_ERROR_CHECK(nrf_drv_twi_tx(&twi_, PRST_SHTC3_ADDR, cmd, 2,
                                 /*no_stop=*/false));
}

void prst_shtc3_init() {
  twi_config_.scl = PRST_SHT3C_SCL_PIN;
  twi_config_.sda = PRST_SHT3C_SDA_PIN;
  twi_config_.frequency = NRF_TWI_FREQ_100K;
}

prst_shtc3_read_t prst_shtc3_read() {
  APP_ERROR_CHECK(nrf_drv_twi_init(&twi_, &twi_config_, NULL, NULL));
  nrf_drv_twi_enable(&twi_);

  // Wake the sensor up.
  write_cmd(PRST_SHTC3_CMD_WAKEUP);
  nrf_delay_ms(1);
  // Request measurement.
  write_cmd(PRST_SHTC3_CMD_MEASURE_TFIRST_LOW_POWER);
  // Read temp and humidity.
  while (nrf_drv_twi_rx(&twi_, PRST_SHTC3_ADDR, buff, 6) != 0) {
    nrf_delay_ms(10);
  }
  // Put the sensor in sleep mode.
  write_cmd(PRST_SHTC3_CMD_SLEEP);

  // Uninit i2c.
  nrf_drv_twi_uninit(&twi_);

  NRF_LOG_INFO("Computing...");
  double temp_c =
      -45 + 175 * ((double)((buff[0] << 8) | buff[1])) / (1 << 16);
  // double humi = ((double)((buff[3] << 8) | buff[4])) / ((1 << 16) - 1);
  uint16_t humi = (buff[3] << 8) | buff[4];

  prst_shtc3_read_t ret = {.temp_millicelcius = temp_c * 1000, .humidity = humi };
  return ret;
}