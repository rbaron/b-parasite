#ifndef _PARASITE_PWM_H_
#define _PARASITE_PWM_H_

namespace parasite {

// This is a simple, single-channel PWM-based square wave generator.
// This only ever works for a single pin. If you call this function
// twice with different pin numbers, nothing good will come out of it.
// I am particularly proud of how well I resisted making this "generic"
// and "reusable".
void SetupSquareWave(int pin_number);
}  // namespace parasite
#endif  // _PARASITE_PWM_H_