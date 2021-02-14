#include <Arduino.h>

#include <cstring>

#include "parasite/adc.h"
#include "parasite/ble.h"
#include "parasite/ble_advertisement_data.h"
#include "parasite/pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSoilAnalogPin = 4;  // AIN2
constexpr int kBattAnalogPin = 3;  // AIN3
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;
constexpr int kSoilMonitorAirVal = 680;
constexpr int kSoilMonitorWaterVal = 60;

const parasite::MACAddr kMACAddr = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
parasite::BLEAdvertiser advertiser(kMACAddr);

parasite::BatteryMonitor batt_monitor(kBattAnalogPin);
parasite::SoilMonitor soil_monitor(kSoilMonitorAirVal, kSoilMonitorWaterVal,
                                   kSoilAnalogPin);

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
  double battery_voltage = batt_monitor.Read();
  Serial.printf("Batt voltage: %f\n", battery_voltage);

  parasite::soil_reading_t soil_reading = soil_monitor.Read();
  Serial.printf("Moisture val: %d, %f%%\n", soil_reading.raw,
                100 * soil_reading.parcent);

  digitalToggle(kLED1Pin);

  updateAdvertisingData(&advertiser, soil_reading, battery_voltage);

  delay(500);
}