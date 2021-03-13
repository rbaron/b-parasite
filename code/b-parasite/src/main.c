#include <stdbool.h>
#include <stdint.h>

#include "app_timer.h"
#include "ble_advdata.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_delay.h"
#include "nrf_drv_rtc.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_soc.h"

// P0.03
// #define LED_PIN 3
#define LED_PIN NRF_GPIO_PIN_MAP(1, 11)

// Environmental sensing
#define SERVICE_UUID 0x181a

#define DEAD_BEEF 0xDEADBEEF

#define NAME "Parasite"
#define NAME_LEN 8

#define SERVICE_DATA_LEN 8
static uint8_t service_data[SERVICE_DATA_LEN];

#define APP_BLE_CONN_CFG_TAG 1

// Seconds between RTC 2 events.
#define COMPARE_COUNTERTIME (1UL)

// RTC0 is used by softdevice, so we need to pick another instance.
const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(2);

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

  m_adv_params.properties.type =
      BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
  m_adv_params.p_peer_addr = NULL;  // Undirected advertisement.
  m_adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
  m_adv_params.interval = NON_CONNECTABLE_ADV_INTERVAL;
  m_adv_params.duration = 0;  // Never time out.

  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)NAME, NAME_LEN);

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
  NRF_LOG_INFO("Advertising started\n.")
}

static void advertising_stop(void) {
  ret_code_t err_code;
  err_code = sd_ble_gap_adv_stop(m_adv_handle);
  APP_ERROR_CHECK(err_code);
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

static void rtc_handler(nrf_drv_rtc_int_type_t int_type) {
  NRF_LOG_INFO("CALLBACK!\n");
  NRF_LOG_FLUSH();
  if (int_type == NRF_DRV_RTC_INT_COMPARE2) {
    nrf_gpio_pin_set(LED_PIN);
    advertising_start();
    nrf_delay_ms(1000);
    advertising_stop();
    nrf_gpio_pin_clear(LED_PIN);
    nrf_drv_rtc_counter_clear(&rtc);
    // We need to re-enable the RTC2 interrupt.
    nrf_drv_rtc_int_enable(&rtc, NRF_RTC_INT_COMPARE2_MASK);
  }
  // This should be disabled and never triggered.
  else if (int_type == NRF_DRV_RTC_INT_TICK) {
  }
}

static void rtc_config(void) {
  uint32_t err_code;

  NRF_LOG_INFO("1!\n"); NRF_LOG_FLUSH();
  // Initialize RTC instance.
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  config.prescaler = 4095;
  err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("2!\n"); NRF_LOG_FLUSH();
  nrf_drv_rtc_tick_disable(&rtc);
  nrf_drv_rtc_overflow_disable(&rtc);
  nrf_drv_rtc_counter_clear(&rtc);

  NRF_LOG_INFO("3!\n"); NRF_LOG_FLUSH();
  // Set compare channel to trigger interrupt after COMPARE_COUNTERTIME
  // seconds.
  err_code = nrf_drv_rtc_cc_set(&rtc, 2, COMPARE_COUNTERTIME * 8, true);
  APP_ERROR_CHECK(err_code);

  NRF_LOG_INFO("4!\n"); NRF_LOG_FLUSH();
  // Power on RTC instance.
  nrf_drv_rtc_enable(&rtc);
}

#define FPU_EXCEPTION_MASK 0x0000009F

static void power_manage(void) {
  __set_FPSCR(__get_FPSCR() & ~(FPU_EXCEPTION_MASK));
  (void)__get_FPSCR();
  NVIC_ClearPendingIRQ(FPU_IRQn);
  nrf_pwr_mgmt_run();
}

int main(void) {
  // Initialize.
  log_init();
  leds_init();
  power_management_init();
  NRF_LOG_INFO("Power inited\n"); NRF_LOG_FLUSH();
  ble_stack_init();
  NRF_LOG_INFO("BLE stack inited\n"); NRF_LOG_FLUSH();
  advertising_init();
  NRF_LOG_INFO("Adv inited\n"); NRF_LOG_FLUSH();
  rtc_config();
  // advertising_start();

  NRF_LOG_FLUSH();

  // Enter main loop.
  for (;;) {
    // sd_power_system_off();
    power_manage();
    // NRF_LOG_INFO("Hello, RTT!\n");
    // NRF_LOG_FLUSH();
    // nrf_delay_ms(500);
  }
}