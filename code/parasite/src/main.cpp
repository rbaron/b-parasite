// #define NRF_LOG_BACKEND_UART_ENABLED 0
// #define NRF_LOG_ENABLED 0

#include <Arduino.h>

#include <cstring>

#include "parasite/adc.h"
#include "parasite/ble.h"
#include "parasite/ble_advertisement_data.h"
#include "parasite/pwm.h"

// variants/feather_nrf52840_express/variant.cpp
constexpr int kLED1Pin = 17;               // P0.28
constexpr int kLED2Pin = 18;               // P0.02
constexpr int kPWMPin = 33;                // 0.09
constexpr int kSoilAnalogPin = 21;         // P0.31, AIN7
constexpr int kBattAnalogPin = 15;         // P0.05, AIN3
constexpr int kBattMonitorEnablePin = 19;  // P0.03, AIN1
constexpr int kDischargeEnablePin = 16;    // P0.30;
constexpr double kPWMFrequency = 500000;

// parasite::SquareWaveGenerator square_wave_generator(kPWMFrequency, kPWMPin);

const parasite::MACAddr kMACAddr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
parasite::BLEAdvertiser advertiser(kMACAddr);

parasite::BatteryMonitor batt_monitor(kBattAnalogPin);
parasite::SoilMonitor soil_monitor(kSoilAnalogPin);

SoftwareTimer timer;

void updateAdvertisingData(parasite::BLEAdvertiser* advertiser,
                           const parasite::soil_reading_t& soil_reading,
                           double battery_voltage) {
  parasite::BLEAdvertisementData data;

  data.SetSoilMoistureRaw(soil_reading.raw);
  data.SetSoilMoisturePercent(soil_reading.parcent);
  data.SetBatteryVoltage(battery_voltage);

  advertiser->SetData(data);

  if (!advertiser->IsRunning()) {
    advertiser->Start();
  }
}

/*
 * * WARNING *
 * To get this callback to work, I had to increase the freeRTOS timer stack in
 * ~/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/freertos/config/FreeRTOSConfig.h
 * #define configTIMER_TASK_STACK_DEPTH                             (1024)
 */
void timer_cb(TimerHandle_t timer_handle) {
  parasite::SquareWaveGenerator square_wave_generator(kPWMFrequency, kPWMPin);

  // digitalToggle(kLED1Pin);

  digitalWrite(kBattMonitorEnablePin, HIGH);
  delay(10);
  double battery_voltage = batt_monitor.Read();
  digitalWrite(kBattMonitorEnablePin, LOW);
  Serial.printf("Batt voltage: %f\n", battery_voltage);

  digitalWrite(kDischargeEnablePin, HIGH);
  square_wave_generator.Start();
  delay(10);
  parasite::soil_reading_t soil_reading = soil_monitor.Read(battery_voltage);
  square_wave_generator.Stop();
  digitalWrite(kDischargeEnablePin, LOW);
  Serial.printf("Moisture val: %d, %f%%\n", soil_reading.raw,
                100 * soil_reading.parcent);

  updateAdvertisingData(&advertiser, soil_reading, battery_voltage);

  // Keep adversiting for 1 second.
  delay(500);
  advertiser.Stop();
}

void setup() {
  // Serial.begin(9600);
  // // pinMode(kLED1Pin, OUTPUT);
  // pinMode(kDischargeEnablePin, OUTPUT);
  // pinMode(kBattMonitorEnablePin, OUTPUT);

  // digitalWrite(kDischargeEnablePin, HIGH);
  // parasite::SquareWaveGenerator square_wave_generator(kPWMFrequency,
  // kPWMPin); square_wave_generator.Start();

  // timer.begin(1000, timer_cb, /*timerID=*/nullptr, /*repeating=*/true);
  // timer.start();

  // Suspend the loop task. Under the hood this is a freeRTOS task set up
  // by the Adafruit_nNRF52_Arduino package.
  // suspendLoop();
  // NRF_UART0->TASKS_STOPTX = 1;
  // NRF_UART0->TASKS_STOPRX = 1;
  // NRF_UART0->ENABLE = 0;

  // NRF_SPI0->ENABLE = 0;
  // __disable_irq()
  // sd_power_system_off();
  // NRF_POWER->SYSTEMOFF = POWER_SYSTEMOFF_SYSTEMOFF_Enter;
}

void loop() {
  Serial.printf("Ok");
  delay(1000);
}