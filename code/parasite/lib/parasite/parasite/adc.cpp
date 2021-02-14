#include "parasite/adc.h"

#include <Arduino.h>

#include <algorithm>

namespace parasite {
namespace {
// Resultion in bits. ADC read values will be mapped to [0, 2^reference].
constexpr int kResolution = 10;

/*
 * Battery monitoring
 */
// Use the internal 0.6 reference with a gain of 1/2.
// This lets us read values in the range [0, 1.2V].
constexpr eAnalogReference kBattADCReference = AR_INTERNAL_1_2;
constexpr double kMaxVoltage = 1.2;
// How many samples will be averaged out.
constexpr uint32_t kOversampling = 32;
// Voltage divider.
constexpr double kBattDividerR1 = 1470;
constexpr double kBattDividerR2 = 470;
constexpr double kBattDividerFactor =
    (kBattDividerR1 + kBattDividerR2) / kBattDividerR2;
}  // namespace

double BatteryMonitor::Read() {
  analogOversampling(kOversampling);
  analogReference(kBattADCReference);
  int batt_val = analogRead(pin_);
  double v_in = kMaxVoltage * batt_val / (1 << kResolution);
  return kBattDividerFactor * v_in;
}

soil_reading_t SoilMonitor::Read() {
  analogOversampling(kOversampling);
  // Set up the analog reference to be VDD. This allows us to cancel out
  // the effect of the battery discharge across time, since the RC circuit
  // also depends linearly on VDD.
  analogReference(AR_VDD4);
  int raw = analogRead(pin_);
  double percentage =
      static_cast<double>(raw - air_val_) / (water_val_ - air_val_);
  return {raw, std::max(0.0, std::min(1.0, percentage))};
}

}  // namespace parasite