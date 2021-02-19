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

constexpr double kDiodeDrop = 0.6;

// We need the sensor to behave well as the battery discharge. I plan on
// using a CR2032 coin cell, whose voltage ranges from roughly 3.0 (charged)
// to 2.0V (discharged). This is the input range for Vdd, so all of our
// analog-to-digital (ADC) measurements are somewhat relative to this varying
// voltage. The soil sensor itself is at heart an RC circuit: it charges to
// 0.63Vcc within a time constant (RC). More importantly, within a cycle, the
// final charge is proportional to Vcc. If we sample this with our ADC, and use
// a sampling mode that is relative to Vcc (AR_VDD4 here), the final voltage
// should "cancel out", keeping the sampled value constant as we vary Vcc. In
// other words, in theory, the raw value returned from the ADC should remain
// constant as I keep the sensor in the same state and vary the input voltage.
// In practice, I'm seeing a small drift in the raw sampled values. This could
// be due to some capacitance or lower transistor response times in the fast
// discharge circuit.
// I captured some (Vcc, raw adc sample) pairs and did a linear regression to
// correct for these values when the sensor is out in the air and in the water.
// This should mitigate the drifting issue enough.
// In reality, I don't think leaving this correction out would be a big deal. I
// expect the battery to discharge over a period of a few months, and I'd expect
// plants to be watered with the period of days/couple of weeks. This drift
// wouldn't be noticeable in that time scale, but since we can do better, why
// not?
// Some assumptions for these hardcoded values:
// - Sampling resolution of 10 bits!
// - Input (vdd) is in the range 2.0 - 3.0V
double GetRawValAir(double vdd) { return vdd * 94.29 + 308.95; }
double GetRawValWater(double vdd) { return vdd * 36.02 - 57.1; }

}  // namespace

double BatteryMonitor::Read() {
  analogOversampling(kOversampling);
  analogReference(kBattADCReference);
  int batt_val = analogRead(pin_);
  double v_in = kMaxVoltage * batt_val / (1 << kResolution);
  return kBattDividerFactor * v_in;
}

soil_reading_t SoilMonitor::Read(double vdd) {
  analogOversampling(kOversampling);
  // Set up the analog reference to be VDD. This allows us to cancel out
  // the effect of the battery discharge across time, since the RC circuit
  // also depends linearly on VDD.
  analogReference(AR_VDD4);
  int raw = analogRead(pin_);
  double v_out_corr = (vdd * raw) / 1023 + 0.6;
  int raw_corr = v_out_corr / vdd * 1023;

  double air = GetRawValAir(vdd);
  double water = GetRawValWater(vdd);
  double percentage = static_cast<double>(raw - air) / (water - air);
  // Serial.printf(
  //     "vdd: %f, raw: %d, raw_corr: %d, v_out_corr: %f, percentage: %f\n", vdd,
  //     raw, raw_corr, v_out_corr, 100 * percentage);
  return {raw, std::max(0.0, std::min(1.0, percentage))};
}

}  // namespace parasite