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
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

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

  // We setup the analog reference to be VDD. This allows us to cancel out
  // the effect of the battery discharge across time, since the RC circuit
  // also depends linearly on VDD.
  // TODO(rbaron): empirically prove/disprove this.
  analogReference(AR_VDD4);
}

void loop() {
  int sens_val = analogRead(kSensAnalogPin);
  Serial.printf("Val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  updateAdvertisingData(&advertiser, sens_val);
  delay(500);
}