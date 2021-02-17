#include <Arduino.h>

#include <cstring>

#include "parasite/adc.h"
#include "parasite/ble.h"
#include "parasite/ble_advertisement_data.h"
#include "parasite/pwm.h"

// variants/feather_nrf52840_express/variant.cpp
constexpr int kLED1Pin = 17;             // P0.28
constexpr int kLED2Pin = 18;             // P0.02
constexpr int kPWMPin = 33;              // 0.09
constexpr int kSoilAnalogPin = 21;       // P0.31, AIN7
constexpr int kBattAnalogPin = 15;       // P0.05, AIN3
constexpr int kDischargeEnablePin = 16;  // P0.30;
constexpr double kPWMFrequency = 500000;
constexpr int kSoilMonitorAirVal = 680;
constexpr int kSoilMonitorWaterVal = 60;

// parasite::SquareWaveGenerator square_wave_generator(kPWMFrequency, kPWMPin);

const parasite::MACAddr kMACAddr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
parasite::BLEAdvertiser advertiser(kMACAddr);

parasite::BatteryMonitor batt_monitor(kBattAnalogPin);
parasite::SoilMonitor soil_monitor(kSoilMonitorAirVal, kSoilMonitorWaterVal,
                                   kSoilAnalogPin);

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

  digitalToggle(kLED1Pin);

  square_wave_generator.Start();
  digitalWrite(kDischargeEnablePin, HIGH);

  parasite::soil_reading_t soil_reading = soil_monitor.Read();
  // Serial.printf("Moisture val: %d, %f%%\n", soil_reading.raw,
  //               100 * soil_reading.parcent);

  square_wave_generator.Stop();
  digitalWrite(kDischargeEnablePin, LOW);

  double battery_voltage = batt_monitor.Read();
  // Serial.printf("Batt voltage: %f\n", battery_voltage);

  updateAdvertisingData(&advertiser, soil_reading, battery_voltage);

  // Keep adversiting for 1 second.
  delay(1000);

  advertiser.Stop();

  // TODO(rbaron): what else can we turn off to save battery?
}

void setup() {
  Serial.begin(9600);
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kDischargeEnablePin, OUTPUT);

  timer.begin(2000, timer_cb, /*timerID=*/nullptr, /*repeating=*/true);
  timer.start();

  // Suspend the loop task. Under the hood this is a freeRTOS task set up
  // by the Adafruit_nNRF52_Arduino package.
  suspendLoop();
}

void loop() {}