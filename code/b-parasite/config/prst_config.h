#ifndef _PRST_CONFIG_H_
#define _PRST_CONFIG_H_

#include "nrf_gpio.h"

// Built-in LED.
#define PRST_LED_PIN NRF_GPIO_PIN_MAP(1, 11)

// Deep sleep.
#define PRST_DEEP_SLEEP_IN_SECONDS 2

// BLE.
#define PRST_BLE_ADV_NAME "Prst"

// PWM.
#define PRST_PWM_PIN NRF_GPIO_PIN_MAP(0, 29)

#endif  // _PRST_CONFIG_H_