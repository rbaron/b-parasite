#ifndef _PRST_SHT3C_H_
#define _PRST_SHT3C_H_

#include <nrf_gpio.h>

#define PRST_SHT3C_SDA_PIN NRF_GPIO_PIN_MAP(0, 24)
#define PRST_SHT3C_SCL_PIN NRF_GPIO_PIN_MAP(0, 13)

// Values from the SHTC3 datasheet.
#define PRST_SHTC3_ADDR 0x70
#define PRST_SHTC3_CMD_SLEEP 0xb098
#define PRST_SHTC3_CMD_WAKEUP 0x3517
#define PRST_SHTC3_CMD_MEASURE_TFIRST_LOW_POWER 0x609c
#define PRST_SHTC3_CMD_MEASURE_TFIRST_NORMAL 0x7866

typedef struct prst_shtc3_values {
  // Temperature in degrees Celsius.
  float temp_celsius;
  // Relative humidity, from 0 to 2^16.
  uint16_t humidity;
} prst_shtc3_read_t;

void prst_shtc3_init();
prst_shtc3_read_t prst_shtc3_read();

#endif  // _PRST_SHT3C_H_