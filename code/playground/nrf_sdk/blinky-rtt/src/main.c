#include <stdbool.h>
#include <stdint.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define LED_PIN 3

int main(void) {
  nrf_gpio_cfg_output(LED_PIN);
  NRF_LOG_INIT(/*timestamp_func=*/NULL);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
  while (true) {
    nrf_gpio_pin_toggle(LED_PIN);
    NRF_LOG_INFO("Hello, RTT!\n");
    NRF_LOG_FLUSH();
    nrf_delay_ms(500);
  }
}