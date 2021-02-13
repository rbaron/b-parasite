#include <Arduino.h>
#include <bluefruit.h>

#include <cstring>

#include "parasite/ble.h"
#include "parasite/pwm.h"

constexpr int kLED1Pin = 17;
constexpr int kLED2Pin = 18;
constexpr int kPWMPin = 19;
constexpr int kSensAnalogPin = 4;  // AIN2
constexpr int kDischargeEnablePin = 16;
constexpr double kPWMFrequency = 500000;

constexpr int kManufacturerDataLen = 3;

// We can have at most 31 bytes here.
uint8_t advertisement_data[] = {
    9, // Length of name.
    BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
    'P', 'a', 'r', 'a', 's', 'i', 't', 'e',
    7, // Length of the service data.
    BLE_GAP_AD_TYPE_SERVICE_DATA,
    0x1a, 0x18,  // Environment sensor service UUID.
    0x00, 0x00,  // Raw soil humidity.
    0x00, 0x00,  // Percentage soil humidity.
};
constexpr size_t kRawSoilMoistureOffset = 14;
constexpr size_t kPercentSoilMoistureOffset = kRawSoilMoistureOffset + 2;
constexpr size_t kTemperatureOfsset = kRawSoilMoistureOffset + 4;

ble_gap_addr_t kGAPAddr{
    1,
    BLE_GAP_ADDR_TYPE_PUBLIC,
    // This is the "reverse" order in comparison that the colon-separated
    // human-readable MAC addresses.
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06},
};

void setupAdvertising() {
  Bluefruit.begin(1, 1);
  Bluefruit.setName("Parasite");
  Bluefruit.setAddr(&kGAPAddr);
}

void updateAdvertisingData(int moisture_level) {
  uint16_t packed_raw_moisture = moisture_level;
  advertisement_data[kRawSoilMoistureOffset] = packed_raw_moisture >> 8;
  advertisement_data[kRawSoilMoistureOffset + 1] = packed_raw_moisture & 0xff;
  // manufacturer_data[1] = 0xff;
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  // Bluefruit.Advertising.addName();
  // Bluefruit.Advertising.addManufacturerData(manufacturer_data,
  //                                           kManufacturerDataLen);
  Bluefruit.Advertising.setData(advertisement_data, sizeof(advertisement_data));
  Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
  Bluefruit.Advertising.start(0);  // 0 = Don't stop advertising after n seconds
}

void setup() {
  Serial.begin(9600);
  pinMode(kLED1Pin, OUTPUT);
  pinMode(kDischargeEnablePin, OUTPUT);
  parasite::SetupSquareWave(kPWMFrequency, kPWMPin);
  digitalWrite(kDischargeEnablePin, HIGH);

  analogReference(AR_VDD4);

  setupAdvertising();

  Serial.println("Will advertise with MAC:");
  for (const auto byte : kGAPAddr.addr) {
    Serial.printf("0x%02x ", byte);
  }
  Serial.println();
}

void loop() {
  int sens_val = analogRead(kSensAnalogPin);
  Serial.printf("Val: %d\n", sens_val);
  digitalToggle(kLED1Pin);
  updateAdvertisingData(sens_val);
  delay(500);
}
