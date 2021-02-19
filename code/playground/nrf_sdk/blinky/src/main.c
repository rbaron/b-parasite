#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"

#define LED_PIN 3

int main(void) {
  nrf_gpio_cfg_output(LED_PIN);
  while (true) {
    nrf_gpio_pin_toggle(LED_PIN);
    nrf_delay_ms(500);
  }
}