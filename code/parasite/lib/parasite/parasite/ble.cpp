#include "ble.h"

#include <bluefruit.h>

namespace parasite {
namespace {}

void BLEAdvertiser::SetData(BLEAdvertisementData data) {
  data_ = std::move(data);
  if (is_running_) {
    Bluefruit.Advertising.stop();
  }
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.setData(data_.GetRawData(), data_.GetDataLen());
  if (is_running_) {
    Bluefruit.Advertising.start(0);
  }
}

void BLEAdvertiser::Init() {
  ble_gap_addr_t addr;
  addr.addr_id_peer = 1;
  addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
  std::copy(std::begin(mac_addr_), std::end(mac_addr_), std::begin(addr.addr));
  Bluefruit.begin(1, 1);
  Bluefruit.setName("Parasite");
  Bluefruit.setAddr(&addr);
  is_initialized_ = true;
}

void BLEAdvertiser::Start() {
  if (!is_initialized_) {
    Init();
  }
  Bluefruit.Advertising.setInterval(32, 244);  // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);    // number of seconds in fast mode
  Bluefruit.Advertising.start(0);  // 0 = Don't stop advertising after n seconds
  is_running_ = true;
}

void BLEAdvertiser::Stop() {
  Bluefruit.Advertising.stop();
  is_running_ = false;
}

}  // namespace parasite