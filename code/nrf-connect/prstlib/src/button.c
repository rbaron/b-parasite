#include "prstlib/button.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "prstlib/led.h"
#include "prstlib/macros.h"

LOG_MODULE_REGISTER(button, CONFIG_PRSTLIB_LOG_LEVEL);

static struct k_work_delayable button_pressed_delayable;

static struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

static struct gpio_callback cb_data;

static prst_button_callback_t user_callback = NULL;

static void maybe_call_user_callback(prst_button_t button, bool is_active) {
  if (user_callback != NULL) {
    user_callback(button, is_active);
  } else {
    LOG_WRN("No user callback registered for button %d", button);
  }
}

static void button_pressed_cb(struct k_work *work) {
  int button_state = prst_button_poll(PRST_BUTTON_SW1);
  if (button_state < 0) {
    LOG_ERR("Failed to poll button");
    return;
  }
  return maybe_call_user_callback(PRST_BUTTON_SW1, button_state);
}

static void button_pressed_isr(const struct device *dev, struct gpio_callback *cb,
                               uint32_t pins) {
  k_work_reschedule(&button_pressed_delayable, K_MSEC(10));
}

int prst_button_init() {
  RET_IF_ERR(!device_is_ready(button.port));
  RET_IF_ERR(gpio_pin_configure_dt(&button, GPIO_INPUT));
  return 0;
}

int prst_button_register_callback(prst_button_callback_t callback) {
  k_work_init_delayable(&button_pressed_delayable, button_pressed_cb);
  // EDGE interrupts seem to consume more power than LEVEL ones.
  // For GPIO_INT_EDGE_BOTH: 16 uA idle.
  // For GPIO_INT_LEVEL_ACTIVE: 3 uA idle.
  // Related issue:
  // https://github.com/zephyrproject-rtos/zephyr/issues/28499
  // Apparently sense-edge-mask brings the power consumption down to
  // 3 uA for EDGE interrupts too.
  RET_IF_ERR(gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH));
  gpio_init_callback(&cb_data, button_pressed_isr, BIT(button.pin));
  RET_IF_ERR(gpio_add_callback(button.port, &cb_data));
  user_callback = callback;
  return 0;
}

int prst_button_poll(prst_button_t prst_button) {
  RET_CHECK(prst_button == PRST_BUTTON_SW1, "Invalid button");
  return gpio_pin_get_dt(&button);
}
