#ifndef _PARASITE_BLE_ADVERTISEMENT_DATA_H_
#define _PARASITE_BLE_ADVERTISEMENT_DATA_H_

// #include <string>
#include <ble_gap.h>

#include <cstddef>

namespace parasite {

constexpr size_t kAdvertisementDataLen = 21;
constexpr size_t kRawSoilMoistureOffset = 14;
constexpr size_t kPercentSoilMoistureOffset = kRawSoilMoistureOffset + 2;
constexpr size_t kBatteryVoltageOffset = kRawSoilMoistureOffset + 4;

class BLEAdvertisementData {
 public:
  void SetSoilMoistureRaw(int soil_moisture_raw);
  void SetSoilMoisturePercent(double soil_moisture_percent);
  void SetBatteryVoltage(double battery_voltage);
  const uint8_t* GetRawData() const { return data_; }
  size_t GetDataLen() const { return kAdvertisementDataLen; }

 private:
  uint8_t data_[kAdvertisementDataLen] = {
      9,  // Length of name + data type.
      BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME,
      'P',
      'a',
      'r',
      'a',
      's',
      'i',
      't',
      'e',
      10,  // Length of the service data + data type.
      BLE_GAP_AD_TYPE_SERVICE_DATA,
      0x1a,
      0x18,  // Environment sensor service UUID (0x181a).
      0x00,
      0x00,  // Raw soil humidity (2 bytes).
      0x00,
      0x00,  // Percentage soil humidity (2 bytes).
      0x00,
      0x00,  // Battery voltage (2 bytes representing millivolts).
      0x00,
  };
};

}  // namespace parasite
#endif  // _PARASITE_BLE_ADVERTISEMENT_DATA_H_