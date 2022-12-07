#ifndef _PRST_LED_H_
#define _PRST_LED_H_

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "prstlib/macros.h"

#define PRST_LED_FLASH_PERIOD_MS 400

extern struct gpio_dt_spec led;

int prst_led_init();

static inline int prst_led_on() {
  return gpio_pin_set_dt(&led, 1);
}

static inline int prst_led_off() {
  return gpio_pin_set_dt(&led, 0);
}

static inline int prst_led_toggle() {
  return gpio_pin_toggle_dt(&led);
}

static inline int prst_led_flash(int times) {
  LOG_MODULE_DECLARE(led, LOG_LEVEL_DBG);
  RET_IF_ERR(prst_led_off());
  for (int i = 0; i < 2 * times; i++) {
    RET_IF_ERR(prst_led_toggle());
    k_msleep(PRST_LED_FLASH_PERIOD_MS / 2);
  }
  return 0;
}

#endif  // _PRST_LED_H_