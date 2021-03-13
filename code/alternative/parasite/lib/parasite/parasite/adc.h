#ifndef _PARASITE_ADC_H_
#define _PARASITE_ADC_H_

namespace parasite {

class BatteryMonitor {
 public:
  explicit BatteryMonitor(int pin) : pin_(pin) {}
  double Read();

 private:
  const int pin_;
};

struct soil_reading_t {
  int raw;
  double parcent;
};

class SoilMonitor {
 public:
  explicit SoilMonitor(int pin) : pin_(pin) {}

  soil_reading_t Read(double vdd);

 private:
  // Pin the analog signal is connected to.
  const int pin_;
};
}  // namespace parasite
#endif  // _PARASITE_ADC_H_