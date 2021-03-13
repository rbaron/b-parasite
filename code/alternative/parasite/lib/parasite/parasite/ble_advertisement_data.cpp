#include "ble_advertisement_data.h"

#include <algorithm>

namespace parasite {

void BLEAdvertisementData::SetSoilMoistureRaw(int soil_moisture_raw) {
  uint16_t packed_value = soil_moisture_raw;
  data_[kRawSoilMoistureOffset] = packed_value >> 8;
  data_[kRawSoilMoistureOffset + 1] = packed_value & 0xff;
}

void BLEAdvertisementData::SetSoilMoisturePercent(
    double soil_moisture_percent) {
  // TODO: 1.0 * MAX is too close to overflow for comfort?
  uint16_t packed_value = ((1 << 16) - 1) * soil_moisture_percent;
  data_[kPercentSoilMoistureOffset] = packed_value >> 8;
  data_[kPercentSoilMoistureOffset + 1] = packed_value & 0xff;
}

void BLEAdvertisementData::SetBatteryVoltage(double battery_voltage) {
  uint16_t packed_value = 1000 * battery_voltage;
  data_[kBatteryVoltageOffset] = packed_value >> 8;
  data_[kBatteryVoltageOffset + 1] = packed_value & 0xff;
}

}  // namespace parasite