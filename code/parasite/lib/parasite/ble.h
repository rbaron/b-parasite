#ifndef _PARASITE_BLE_H_
#define _PARASITE_BLE_H_

#include <string>

namespace parasite {

class BLEAdvertiser {
 public:
  void Advertise(std::string name, double moisture) {}
}

}  // namespace parasite
#endif  // _PARASITE_BLE_H_