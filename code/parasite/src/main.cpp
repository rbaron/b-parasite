#include <Arduino.h>

// #include "ble.h"
#include "pwm.h"

#include "nrf_sdh_ble.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSensAnalogPin = 4;  // AIN2
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

// static void ble_stack_init(void) {
//   ret_code_t err_code;

//   err_code = nrf_sdh_enable_request();
//   APP_ERROR_CHECK(err_code);

//   // Configure the BLE stack using the default settings.
//   // Fetch the start address of the application RAM.
//   uint32_t ram_start = 0;
//   err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
//   APP_ERROR_CHECK(err_code);

//   // Enable BLE stack.
//   err_code = nrf_sdh_ble_enable(&ram_start);
//   APP_ERROR_CHECK(err_code);
// }

void setup() {
  Serial.begin(9600);
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kDischargeEnablePin, OUTPUT);
  parasite::SetupSquareWave(kPWMFrequency, kPWMPin);
  digitalWrite(kDischargeEnablePin, HIGH);

  analogReference(AR_VDD4);
}

void loop() {
  int sens_val = analogRead(kSensAnalogPin);
  Serial.printf("Val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  delay(500);
}