#include <prstlib/adc.h>
#include <prstlib/button.h>
#include <prstlib/led.h>
#include <prstlib/macros.h>
#include <prstlib/sensors.h>
#include <prstlib/shtc3.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void button_pressed_cb(prst_button_t button, bool is_active) {
  if (is_active) {
    LOG_INF("Button pressed (debounced)");
    prst_led_on();
  } else {
    LOG_INF("Button released (debounced)");
    prst_led_off();
  }
}

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());

  RET_IF_ERR(prst_led_flash(2));

  prst_sensors_t sensors;
  // Read the sensors just to ensure they'll be put to a low
  // power mode afterward.
  RET_IF_ERR(prst_sensors_read_all(&sensors));

  int initial_button_state = prst_button_poll(PRST_BUTTON_SW1);
  RET_CHECK(initial_button_state >= 0, "Failed to poll button");
  LOG_INF("Initial button state: %s", initial_button_state ? "active" : "inactive");

  RET_IF_ERR(prst_button_register_callback(button_pressed_cb));

  RET_IF_ERR(prst_led_flash(2));

  while (true) {
    LOG_INF("Main loop.");
    k_sleep(K_FOREVER);
  }
  return 0;
}
