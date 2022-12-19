#include "prstlib/button.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "prstlib/led.h"
#include "prstlib/macros.h"

LOG_MODULE_REGISTER(button, CONFIG_PRSTLIB_LOG_LEVEL);

static struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

static struct gpio_callback cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb,
                           uint32_t pins) {
  LOG_INF("Button pressed");
  prst_led_toggle();
}

int prst_button_init() {
  RET_IF_ERR(!device_is_ready(button.port));
  RET_IF_ERR(gpio_pin_configure_dt(&button, GPIO_INPUT));
  // EDGE interrupts consume more power! Just use a LEVEL one.
  RET_IF_ERR(gpio_pin_interrupt_configure_dt(&button, GPIO_INT_LEVEL_ACTIVE));
  gpio_init_callback(&cb_data, button_pressed, BIT(button.pin));
  RET_IF_ERR(gpio_add_callback(button.port, &cb_data));
  return 0;
}