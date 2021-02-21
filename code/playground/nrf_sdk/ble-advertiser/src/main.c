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
/** @file
 *
 * @defgroup ble_sdk_app_beacon_main main.c
 * @{
 * @ingroup ble_sdk_app_beacon
 * @brief Beacon Transmitter Sample Application main file.
 *
 * This file contains the source code for an Beacon transmitter sample
 * application.
 */

#include <stdbool.h>
#include <stdint.h>

#include "app_timer.h"
#include "ble_advdata.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_soc.h"

// P0.03
#define LED_PIN 3

// Environmental sensing
#define SERVICE_UUID 0x181a

#define DEAD_BEEF 0xDEADBEEF

#define NAME "Parasite"
#define NAME_LEN 8

#define APP_BLE_CONN_CFG_TAG 1

static ble_gap_adv_params_t m_adv_params; /**< Parameters to be passed to the
                                             stack when starting advertising. */
static uint8_t m_adv_handle =
    BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an
                                       advertising set. */

#define NON_CONNECTABLE_ADV_INTERVAL MSEC_TO_UNITS(100, UNIT_0_625_MS)

static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

static ble_gap_adv_data_t m_adv_data = {
    .adv_data = {.p_data = m_enc_advdata, .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX},
    .scan_rsp_data = {.p_data = NULL, .len = 0

    }};


#define SERVICE_DATA_LEN 8
static uint8_t service_data[SERVICE_DATA_LEN];

void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
  app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

static void advertising_init(void) {

  uint32_t err_code;
  ble_advdata_t advdata;
  // Build and set advertising data.
  memset(&advdata, 0, sizeof(advdata));

  uint8_t flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

  ble_advdata_service_data_t advdata_service_data;
  advdata_service_data.service_uuid = SERVICE_UUID;
  advdata_service_data.data.p_data = service_data;
  advdata_service_data.data.size = SERVICE_DATA_LEN;


  advdata.name_type = BLE_ADVDATA_FULL_NAME;
  advdata.flags = flags;

  advdata.p_service_data_array = &advdata_service_data;
  advdata.service_data_count = 1;

  // Initialize advertising parameters (used when starting advertising).
  memset(&m_adv_params, 0, sizeof(m_adv_params));

  m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
  m_adv_params.p_peer_addr = NULL;  // Undirected advertisement.
  m_adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
  m_adv_params.interval = NON_CONNECTABLE_ADV_INTERVAL;
  m_adv_params.duration = 0;  // Never time out.

  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *) NAME, NAME_LEN);

  err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data,
                                &m_adv_data.adv_data.len);
  APP_ERROR_CHECK(err_code);

  err_code =
      sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
  APP_ERROR_CHECK(err_code);
}

static void advertising_start(void) {
  ret_code_t err_code;

  err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
  APP_ERROR_CHECK(err_code);

  nrf_gpio_pin_set(LED_PIN);
}

static void ble_stack_init(void) {
  ret_code_t err_code;

  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Configure the BLE stack using the default settings.
  // Fetch the start address of the application RAM.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable BLE stack.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing logging. */
static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_DEFAULT_BACKENDS_INIT();
  NRF_LOG_INFO("Log inited");
}

static void leds_init(void) {
  nrf_gpio_cfg_output(LED_PIN);
  nrf_gpio_pin_toggle(LED_PIN);
  nrf_delay_ms(500);
  nrf_gpio_pin_toggle(LED_PIN);
  nrf_delay_ms(500);
  NRF_LOG_INFO("Leds inited");
}

static void timers_init(void) {
  ret_code_t err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);
}

static void power_management_init(void) {
  ret_code_t err_code;
  err_code = nrf_pwr_mgmt_init();
  APP_ERROR_CHECK(err_code);
}

static void idle_state_handle(void) {
  if (NRF_LOG_PROCESS() == false) {
    nrf_pwr_mgmt_run();
  }
}

/**
 * @brief Function for application main entry.
 */
int main(void) {
  // Initialize.
  log_init();
  timers_init();
  leds_init();
  power_management_init();
  ble_stack_init();
  advertising_init();

  // Start execution.
  NRF_LOG_INFO("Beacon example started.");
  advertising_start();

  // Enter main loop.
  for (;;) {
    idle_state_handle();
    nrf_delay_ms(100);
    NRF_LOG_FLUSH();
  }
}