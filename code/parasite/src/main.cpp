#include <Arduino.h>

#include "pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;

void setup() {
  Serial.begin(9600);
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kLED2Pin, OUTPUT);
  parasite::SetupSquareWave(kPWMPin);
}

void loop() {
  Serial.print("Hello, world\n");
  digitalWrite(kLED1Pin, LOW);
  digitalWrite(kLED2Pin, LOW);
  delay(500);
  digitalWrite(kLED1Pin, HIGH);
  digitalWrite(kLED2Pin, HIGH);
  delay(500);
}