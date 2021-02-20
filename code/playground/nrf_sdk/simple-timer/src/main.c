/**
 * Copyright (c) 2014 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be
 * reverse engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**@file
 * @defgroup nrf_dev_simple_timer_example_main.c
 * @{
 * @ingroup nrf_dev_simple_timer_example
 * @brief Timer example application main file.
 *
 * This file contains the source code for a sample application using timer
 * library. For a more detailed description of the functionality, see the SDK
 * documentation.
 */

#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_simple_timer.h"
#include "boards.h"
#include "nrf_delay.h"

#define LED_PIN 3

void timeout_handler(void* p_context) { nrf_gpio_pin_toggle(LED_PIN); }

/**@brief Function for the Power Management.
 */
static void power_manage(void) {
  // Use directly __WFE and __SEV macros since the SoftDevice is not available.

  // Wait for event.
  __WFE();

  // Clear Event Register.
  __SEV();
  __WFE();
}

int main(void) {
  nrf_gpio_cfg_output(LED_PIN);

  uint32_t err_code = app_simple_timer_init();
  APP_ERROR_CHECK(err_code);

  err_code = app_simple_timer_start(APP_SIMPLE_TIMER_MODE_REPEATED,
                                    timeout_handler, 20000, NULL);
  APP_ERROR_CHECK(err_code);

  for (;;) {
    power_manage();
  }
}
