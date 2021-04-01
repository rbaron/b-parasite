#ifndef _PRST_CONFIG_H_
#define _PRST_CONFIG_H_

#include "nrf_gpio.h"

// Built-in LED.
#define PRST_LED_PIN NRF_GPIO_PIN_MAP(0, 28)

// Deep sleep.
#define PRST_DEEP_SLEEP_IN_SECONDS 300

// Analog to digital converter (ADC).
// Prints out ADC debug info, such as the values read for battery and soil
// moisture.
#define PRST_ADC_BATT_DEBUG 0
#define PRST_ADC_SOIL_DEBUG 0

// BLE.
// Prints out BLE debug info, such as the final encoded advertisement packet.
#define PRST_BLE_DEBUG 0
#define PRST_BLE_PROTOCOL_VERSION 1
#define PRST_BLE_MAC_ADDR_LSB1 0x00
#define PRST_BLE_MAC_ADDR_LSB0 0x07
#define PRST_BLE_ADV_NAME "prst"
// Total time spend advertising.
#define PRST_BLE_ADV_TIME_IN_MS 1000
// Interval between advertising packets.
// From the specs, this value has to be greater or equal 20ms.
#define PRST_BLE_ADV_INTERVAL_IN_MS 30
// Possible values are ..., -8, -4, 0, 4, 8.
#define PRST_BLE_ADV_TX_POWER 8

// PWM.
#define PRST_PWM_PIN NRF_GPIO_PIN_MAP(0, 5)
#define PRST_FAST_DISCH_PIN NRF_GPIO_PIN_MAP(1, 10)

// SHT3C temp/humidity sensor.
#define PRST_SHT3C_DEBUG 0

#endif  // _PRST_CONFIG_H_