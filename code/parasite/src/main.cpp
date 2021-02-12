#include <Arduino.h>

#include "parasite/pwm.h"
#include "parasite/ble.h"

#include <bluefruit.h>

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

void setupAdvertising() {
  Bluefruit.begin(1, 1);
  Bluefruit.setName("Parasite");
}

void updateAdvertisingData(int moisture_level) {
  manufacturer_data[2] = moisture_level & 0xff;
  manufacturer_data[1] = 0xff;
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addManufacturerData(manufacturer_data, kManufacturerDataLen);
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
  // Bluefruit.Advertising.start();

  // ble.setConnectable(false);
  // ble.addLocalAttribute();
}

void loop() {
  int sens_val = analogRead(kSensAnalogPin);
  Serial.printf("Val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  updateAdvertisingData(sens_val);
  delay(500);
}
