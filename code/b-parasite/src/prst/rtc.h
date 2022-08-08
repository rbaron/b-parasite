#ifndef _PRST_RTC_H_
#define _PRST_RTC_H_

#include <stdint.h>

typedef void (*prst_rtc_callback_t)(void);

void prst_rtc_set_callback(prst_rtc_callback_t cb);

void prst_rtc_init();

void prst_rtc_set_timer(uint16_t seconds);

#endif  // _PRST_RTC_H_