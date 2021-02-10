#include <Arduino.h>

#define kLED1Pin 18

void setup() {
  pinMode(LED_RED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.println("Hello, world");
  digitalToggle(LED_RED);
  delay(1000);
}
