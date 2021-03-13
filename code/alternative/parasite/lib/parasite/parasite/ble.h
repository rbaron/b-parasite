#ifndef _PARASITE_BLE_H_
#define _PARASITE_BLE_H_

#include <array>
#include <string>

#include "parasite/ble_advertisement_data.h"

namespace parasite {

using MACAddr = std::array<uint8_t, 6>;

class BLEAdvertiser {
 public:
  explicit BLEAdvertiser(MACAddr mac_addr) : mac_addr_(std::move(mac_addr)) {}
  void SetData(BLEAdvertisementData data);
  void Start();
  void Stop();
  bool IsRunning() const { return is_running_; };

 private:
  bool is_initialized_;
  bool is_running_;
  void Init();
  BLEAdvertisementData data_;
  MACAddr mac_addr_;
};

}  // namespace parasite
#endif  // _PARASITE_BLE_H_