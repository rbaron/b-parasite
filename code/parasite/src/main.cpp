#include <Arduino.h>

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;

void setup() {
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kLED2Pin, OUTPUT);
}

void loop() {
  digitalWrite(kLED1Pin, LOW);
  digitalWrite(kLED2Pin, LOW);
  delay(1000);
  digitalWrite(kLED1Pin, HIGH);
  digitalWrite(kLED2Pin, HIGH);
  delay(1000);
}