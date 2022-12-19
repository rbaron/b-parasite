#include "prstlib/led.h"

#include <zephyr/drivers/gpio.h>

#include "prstlib/macros.h"

LOG_MODULE_REGISTER(led, CONFIG_PRSTLIB_LOG_LEVEL);

struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

int prst_led_init() {
  RET_IF_ERR(!device_is_ready(led.port));
  return gpio_pin_configure_dt(&led, GPIO_OUTPUT);
}