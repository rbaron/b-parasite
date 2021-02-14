#include <Arduino.h>

#include <cstring>
#include <vector>

#include "parasite/ble.h"
#include "parasite/ble_advertisement_data.h"
#include "parasite/pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSensAnalogPin = 4;  // AIN2
constexpr int kBattAnalogPin = 3;  // AIN3
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

constexpr double kBattDividerR1 = 1470;
constexpr double kBattDividerR2 = 470;
constexpr double kBattDividerFactor = (kBattDividerR1 + kBattDividerR2) / kBattDividerR2;

const parasite::MACAddr kMACAddr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
parasite::BLEAdvertiser advertiser(kMACAddr);

void updateAdvertisingData(parasite::BLEAdvertiser *advertiser,
                           int moisture_level) {
  parasite::BLEAdvertisementData data;
  data.SetRawSoilMoisture(moisture_level);

  advertiser->SetData(data);

  if (!advertiser->IsRunning()) {
    advertiser->Start();
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(kLED1Pin, OUTPUT);
  pinMode(kDischargeEnablePin, OUTPUT);

  // Activate the PWM signal.
  parasite::SetupSquareWave(kPWMFrequency, kPWMPin);

  // Enable fast discharge cycle.
  digitalWrite(kDischargeEnablePin, HIGH);
}

void loop() {
  // With a gain of 1/2, we can read the range of [0, 1.2V].
  // I'm using a voltage divider with R1 = 1470, R2 470, so
  // We can read 0 - ~5V.
  // This seems to be working okay, but I need to investigate if making it
  // stiffer (lower R1 and R2) work better.
  analogOversampling(32);
  analogReference(AR_INTERNAL_1_2);
  int batt_val = analogRead(kBattAnalogPin);
  double v_in = 1.2 * batt_val / (1<<10);
  double batt_voltage = kBattDividerFactor * v_in;
  Serial.printf("Batt val: %d, voltage: %f\n", batt_val, batt_voltage);

  // We setup the analog reference to be VDD. This allows us to cancel out
  // the effect of the battery discharge across time, since the RC circuit
  // also depends linearly on VDD.
  analogReference(AR_VDD4);
  int sens_val = analogRead(kSensAnalogPin);
  // Serial.printf("Moisture val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  updateAdvertisingData(&advertiser, sens_val);
  delay(500);
}