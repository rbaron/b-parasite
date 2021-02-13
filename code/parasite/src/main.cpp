#include <Arduino.h>
#include <bluefruit.h>

#include <cstring>

#include "parasite/ble.h"
#include "parasite/pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSensAnalogPin = 4;  // AIN2
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

char manufacturer_data[] = {
    0x01,
    0x02,
    0x03,
};

constexpr int kManufacturerDataLen = 3;

ble_gap_addr_t kGAPAddr{
    1,
    BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
    // This is the "reverse" order in comparison that the colon-separated
    // human-readable MAC addresses.
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
};

void setupAdvertising() {
  Bluefruit.begin(1, 1);
  Bluefruit.setName("Parasite");
  Bluefruit.setAddr(&kGAPAddr);
}

void updateAdvertisingData(int moisture_level) {
  manufacturer_data[2] = moisture_level & 0xff;
  manufacturer_data[1] = 0xff;
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addManufacturerData(manufacturer_data,
                                            kManufacturerDataLen);
  Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
  Bluefruit.Advertising.start(0);  // 0 = Don't stop advertising after n seconds
}

void setup() {
  Serial.begin(9600);
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kDischargeEnablePin, OUTPUT);
  parasite::SetupSquareWave(kPWMFrequency, kPWMPin);
  digitalWrite(kDischargeEnablePin, HIGH);

  analogReference(AR_VDD4);

  setupAdvertising();
}

void loop() {
  int sens_val = analogRead(kSensAnalogPin);
  Serial.printf("Val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  updateAdvertisingData(sens_val);
  delay(500);
}
