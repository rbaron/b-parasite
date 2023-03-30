#ifndef _PRST_BUTTON_H_
#define _PRST_BUTTON_H_

#include <stdbool.h>

typedef enum {
  PRST_BUTTON_SW1 = 0,
} prst_button_t;

typedef void (*prst_button_callback_t)(prst_button_t button, bool is_active);

// Inits button driver.
int prst_button_init();

// Configures ISR and calls callback on debounced button press/release.
int prst_button_register_callback(prst_button_callback_t callback);

// Returns:
// 1 if button is active
// 0 if button is inactive
// -1 on error
int prst_button_poll(prst_button_t prst_button);

#endif  // _PRST_BUTTON_H_
