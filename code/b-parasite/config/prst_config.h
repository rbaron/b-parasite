#ifndef _PRST_CONFIG_H_
#define _PRST_CONFIG_H_

#include "nrf_gpio.h"

// Built-in LED.
#define PRST_LED_PIN NRF_GPIO_PIN_MAP(0, 28)

// Deep sleep.
#define PRST_DEEP_SLEEP_IN_SECONDS 3

// Analog to digital converter (ADC).
// Prints out ADC debug info, such as the values read for battery and soil
// moisture.
#define PRST_ADC_DEBUG 0

// BLE.
#define PRST_BLE_ADV_NAME "prst"
// Prints out BLE debug info, such as the final encoded advertisement packet.
#define PRST_BLE_DEBUG 0

// PWM.
#define PRST_PWM_PIN NRF_GPIO_PIN_MAP(0, 5)
#define PRST_FAST_DISCH_PIN NRF_GPIO_PIN_MAP(1, 10)

// SHT3C temp/humidity sensor.
#define PRST_SHT3C_DEBUG 1

#endif  // _PRST_CONFIG_H_