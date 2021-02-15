#include <Arduino.h>

#define kLED1Pin 28

void setup() {
  pinMode(kLED1Pin, OUTPUT);
  Serial.begin(9600);
}

bool on = false;
void loop() {
  Serial.println("Hello, world");
  digitalWrite(kLED1Pin, on ? LOW : HIGH);
  on = !on;
  delay(500);
}
