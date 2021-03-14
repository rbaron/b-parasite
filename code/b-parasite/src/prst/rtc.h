#ifndef _PRST_RTC_H_
#define _PRST_RTC_H_

typedef void (*prst_rtc_callback_t)(void);

void prst_rtc_set_callback(prst_rtc_callback_t cb);

void prst_rtc_init();

#endif  // _PRST_RTC_H_