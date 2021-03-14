#include "prst/shtc3.h"

#include <app_error.h>
#include <nrf_delay.h>
#include <nrf_drv_twi.h>
#include <nrf_gpio.h>
#include <nrf_log.h>

// #define PRST_SHT3C_DEFAULT_ADDR 0x70
#define PRST_SHT3C_DEFAULT_ADDR 0x44

static const nrf_drv_twi_t twi_ = NRF_DRV_TWI_INSTANCE(0);

static uint8_t buff[6];
static uint8_t tx_data[] = {0x2c, 0x06};

void prst_sht3c_init() {
  nrf_drv_twi_config_t twi_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  twi_config.scl = NRF_GPIO_PIN_MAP(0, 3);
  twi_config.sda = NRF_GPIO_PIN_MAP(0, 28);
  // twi_config.clear_bus_init = true;
  twi_config.frequency = NRF_TWI_FREQ_400K;

  uint32_t err_code;
  // uint8_t tx_data[] = {0x2c, 0x06};
  err_code = nrf_drv_twi_init(&twi_, &twi_config, NULL, NULL);
  APP_ERROR_CHECK(err_code);
  nrf_drv_twi_enable(&twi_);
  nrf_delay_ms(10);
  err_code = nrf_drv_twi_tx(&twi_, PRST_SHT3C_DEFAULT_ADDR, tx_data,
                            sizeof(tx_data), true);
  APP_ERROR_CHECK(err_code);

  nrf_delay_ms(10);

  NRF_LOG_INFO("WILL READ DATA:");
  err_code = nrf_drv_twi_rx(&twi_, PRST_SHT3C_DEFAULT_ADDR, buff, 6);
  NRF_LOG_INFO("OH NO!");
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("READ DATA: \n");
  for (int i = 0; i < 6; i++) {
    NRF_LOG_INFO("0x%d ", buff[i]);
  }
  NRF_LOG_INFO("Okay. \n");
}

void prst_sht3c_read();