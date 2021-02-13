#include "ble_advertisement_data.h"

namespace parasite {

void BLEAdvertisementData::SetRawSoilMoisture(int raw_soil_moisture) {
  uint16_t packed_value = raw_soil_moisture;
  data_[kRawSoilMoistureOffset] = packed_value >> 8;
  data_[kRawSoilMoistureOffset + 1] = packed_value & 0xff;
}

}  // namespace parasite