#include "prst/ble.h"

#include <ble_advdata.h>
#include <ble_gap.h>
#include <nordic_common.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble.h>

#include "prst_config.h"

// Environmental sensing.
#define SERVICE_UUID 0x181a

#define NAME "Prst"

#define SERVICE_DATA_LEN 8

// The connection to configure. We only have the one.
#define PRST_CONN_CFG_TAG 1

#define NON_CONNECTABLE_ADV_INTERVAL MSEC_TO_UNITS(100, UNIT_0_625_MS)

static uint8_t service_data[SERVICE_DATA_LEN];

// Stores the encoded advertisement data. As per BLE spec, 31 bytes max.
static uint8_t encoded_adv_data_[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

// Structure holding high level advertisement data and contains a pointer to
// the actual advertised bytes.
static ble_gap_adv_data_t adv_data_ = {
    .adv_data = {.p_data = encoded_adv_data_,
                 .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX},
    .scan_rsp_data = {.p_data = NULL, .len = 0}};

// NRF supports multiple advertisement sets. This initialization is a request
// for configuring a new one.
static uint8_t adv_handle_ = BLE_GAP_ADV_SET_HANDLE_NOT_SET;

// Advertisement parameters.
static ble_gap_adv_params_t adv_params_;

static void init_advertisement_data() {
  // Build advertisement data.
  ble_advdata_t advdata;
  memset(&advdata, 0, sizeof(advdata));
  // memset(&adv_params_, 0, sizeof(adv_params_));

  uint8_t flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;

  // Service data represents a service UUID (2 bytes), followed by
  // SERVICE_DATA_LEN bytes of data from our sensors.
  ble_advdata_service_data_t advdata_service_data;
  advdata_service_data.service_uuid = SERVICE_UUID;
  advdata_service_data.data.p_data = service_data;
  advdata_service_data.data.size = SERVICE_DATA_LEN;

  advdata.name_type = BLE_ADVDATA_FULL_NAME;
  advdata.flags = flags;

  advdata.p_service_data_array = &advdata_service_data;
  advdata.service_data_count = 1;

  // We'll just broadcast our data, so we disallow connections and scan
  // requests.
  adv_params_.properties.type =
      BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;

  // No particular peer - undirected advertisement.
  adv_params_.p_peer_addr = NULL;
  adv_params_.filter_policy = BLE_GAP_ADV_FP_ANY;
  adv_params_.interval = NON_CONNECTABLE_ADV_INTERVAL;
  adv_params_.duration = 0;  // Never time out.

  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)NAME, strlen(NAME));

  // Encodes the advdata into encoded_adv_data.
  uint32_t err_code = ble_advdata_encode(&advdata, adv_data_.adv_data.p_data,
                                         &adv_data_.adv_data.len);
  APP_ERROR_CHECK(err_code);
}

void prst_ble_init() {
  uint32_t err_code;

  // Enable SoftDevice request.
  err_code = nrf_sdh_enable_request();
  APP_ERROR_CHECK(err_code);

  // Set the default config and get the starting RAM address.
  uint32_t ram_start = 0;
  err_code = nrf_sdh_ble_default_cfg_set(PRST_CONN_CFG_TAG, &ram_start);
  APP_ERROR_CHECK(err_code);

  // Enable SoftDevice.
  err_code = nrf_sdh_ble_enable(&ram_start);
  APP_ERROR_CHECK(err_code);

  init_advertisement_data();
}

void prst_ble_update_adv_data(uint8_t n) {
  uint32_t err_code =
      sd_ble_gap_adv_set_configure(&adv_handle_, &adv_data_, &adv_params_);
  APP_ERROR_CHECK(err_code);
}

void prst_adv_start() {
  ret_code_t err_code;
  err_code = sd_ble_gap_adv_start(adv_handle_, PRST_CONN_CFG_TAG);
  APP_ERROR_CHECK(err_code);
  // NRF_LOG_INFO("Advertising started\n.");
}

void prst_adv_stop() {
  ret_code_t err_code;
  err_code = sd_ble_gap_adv_stop(adv_handle_);
  APP_ERROR_CHECK(err_code);
}