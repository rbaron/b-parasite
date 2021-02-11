#include <Arduino.h>

#include "pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSensAnalogPin = 4;  // AIN2
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

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