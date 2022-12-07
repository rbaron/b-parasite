#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#define SLEEP_TIME_MS 1000

static const struct gpio_dt_spec led =
    GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

void main(void) {
  int ret;

  if (!device_is_ready(led.port)) {
    return;
  }

  ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    return;
  }

  while (1) {
    ret = gpio_pin_toggle_dt(&led);
    if (ret < 0) {
      return;
    }
    k_msleep(SLEEP_TIME_MS);
  }
}
