#ifndef _PRST_ZB_DOUBLE_RESET_DETECTOR_H_
#define _PRST_ZB_DOUBLE_RESET_DETECTOR_H_

typedef int (*prst_double_reset_callback_t)();

int prst_detect_double_reset(prst_double_reset_callback_t on_double_reset);

#endif  // _PRST_ZB_DOUBLE_RESET_DETECTOR_H_