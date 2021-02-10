#include "pwm.h"

#include <Arduino.h>
#include <HardwarePWM.h>
#include <nrf.h>

namespace parasite {

namespace {
// nRF52 PWM specs/guide:
// https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.nrf52832.ps.v1.1%2Fpwm.html
// Adafruit's HardwarePWM wapper:
// https://github.com/adafruit/Adafruit_nRF52_Arduino/blob/a86fd9f36c88db96f676cead1e836377a37c7b05/cores/nRF5/HardwarePWM.cpp

// No scaling. The PWM counter will increase with a frequency of 16MHz.
constexpr unsigned long kPWMFrequencyPrescale = PWM_PRESCALER_PRESCALER_DIV_1;
// In conjunction with the PWM clock frequency, kPWMMaxCounter defines the
// frequency of the square wave.
constexpr int kPWMMaxCounter = 32;
// Since we want a duty cycle of 0.5, we flip the PVM output when the counter
// reaches kPWMMaxCounter / 2/;
constexpr int kPWMFlipCount = kPWMMaxCounter;

}  // namespace

void SetupSquareWave(int pin_number) {
  HwPWM0.addPin(pin_number);
  HwPWM0.setClockDiv(PWM_PRESCALER_PRESCALER_DIV_1);
  HwPWM0.setMaxValue(kPWMMaxCounter);
  HwPWM0.writePin(pin_number, kPWMFlipCount);
  HwPWM0.begin();
}

}  // namespace parasite