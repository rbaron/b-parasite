#include "prst/ble.h"

#include <ble_advdata.h>
#include <ble_gap.h>
#include <nordic_common.h>
#include <nrf_log.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble.h>

#include "prst_config.h"

// We need to pick a service UUID for broadcasting our sensor data.
// 0x181a is defined as "environmental sensing", which seems appopriate.
#define SERVICE_UUID 0x181a

// The connection to configure. We only have the one.
#define PRST_CONN_CFG_TAG 1

#define NON_CONNECTABLE_ADV_INTERVAL \
  MSEC_TO_UNITS(PRST_BLE_ADV_INTERVAL_IN_MS, UNIT_0_625_MS)

// Sensor data payload that will go into the advertisement message.
// We have a maximum of 20 bytes to play with here.
#define SERVICE_DATA_LEN 12
static uint8_t service_data[SERVICE_DATA_LEN];

// Stores the encoded advertisement data. As per BLE spec, 31 bytes max.
static uint8_t encoded_adv_data_[BLE_GAP_ADV_SET_DATA_SIZE_MAX];

// Structure holding high level advertisement data and contains a pointer to
// the actual encoded advertised bytes.
static ble_gap_adv_data_t gap_adv_data_ = {
    .adv_data = {.p_data = encoded_adv_data_,
                 .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX},
    .scan_rsp_data = {.p_data = NULL, .len = 0}};

// We'll put our sensor data inside an advertisement service.
static ble_advdata_service_data_t advdata_service_data_ = {
    .service_uuid = SERVICE_UUID,
    .data = {
        .p_data = service_data,
        .size = SERVICE_DATA_LEN,
    }};

// Holds the service data to be broadcasted. The contents of this struct
// will be encoded into gap_adv_data.
// Warning: do not update this while advertising.
static ble_advdata_t adv_data_ = {
    .name_type = BLE_ADVDATA_FULL_NAME,
    .flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED,
    .p_service_data_array = &advdata_service_data_,
    .service_data_count = 1,
};

// NRF supports multiple advertisement sets. This initialization is a request
// for configuring a new one.
static uint8_t adv_handle_ = BLE_GAP_ADV_SET_HANDLE_NOT_SET;

// Advertisement parameters.
static ble_gap_adv_params_t adv_params_;

// Stores the MAC address & type.
// We're using a random static MAC address, which has the following constraints:
// 1. Two most significant bits are set to 1;
// 2. The remaining bits should not _all_ be set to 0;
// 2. The remaining bits should not _all_ be set to 1;
static const ble_gap_addr_t gap_addr_ = {
    .addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC,
    // Least significant bytes first.
    .addr = {PRST_BLE_MAC_ADDR_LSB0, PRST_BLE_MAC_ADDR_LSB1, 0xca, 0xf0, 0xca,
             0xf0}};

static void init_advertisement_data() {
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
  sd_ble_gap_device_name_set(&sec_mode, (const uint8_t *)PRST_BLE_ADV_NAME,
                             strlen(PRST_BLE_ADV_NAME));

  uint32_t err_code =
      sd_ble_gap_adv_set_configure(&adv_handle_, &gap_adv_data_, &adv_params_);
  APP_ERROR_CHECK(err_code);

  // Four bits for the protocol version.
  service_data[0] = (PRST_BLE_PROTOCOL_VERSION << 4) & 0xf0;

  // Bytes 10 and 11 contain the last two bytes of the MAC address.
  service_data[10] = PRST_BLE_MAC_ADDR_LSB1;
  service_data[11] = PRST_BLE_MAC_ADDR_LSB0;
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

  APP_ERROR_CHECK(sd_ble_gap_addr_set(&gap_addr_));

  init_advertisement_data();
}

void prst_ble_update_adv_data(uint16_t batt_millivolts,
                              uint16_t temp_millicelcius, uint16_t humidity,
                              uint16_t soil_moisture, uint8_t run_counter) {
  // 4 bits for a small wrap-around counter for deduplicating messages on the
  // receiver.
  service_data[1] = run_counter & 0x0f;

  service_data[2] = batt_millivolts >> 8;
  service_data[3] = batt_millivolts & 0xff;

  service_data[4] = temp_millicelcius >> 8;
  service_data[5] = temp_millicelcius & 0xff;

  service_data[6] = humidity >> 8;
  service_data[7] = humidity & 0xff;

  service_data[8] = soil_moisture >> 8;
  service_data[9] = soil_moisture & 0xff;

  // Encodes adv_data_ into .gap_adv_data_.
  uint32_t err_code = ble_advdata_encode(
      &adv_data_, gap_adv_data_.adv_data.p_data, &gap_adv_data_.adv_data.len);
  APP_ERROR_CHECK(err_code);

#if PRST_BLE_DEBUG
  NRF_LOG_INFO("[ble] Encoded BLE adv packet:");
  for (int i = 0; i < sizeof(encoded_adv_data_); i++) {
    NRF_LOG_INFO("[ble] 0x%02x", encoded_adv_data_[i]);
  }
#endif
}

void prst_adv_start() {
  APP_ERROR_CHECK(sd_ble_gap_adv_start(adv_handle_, PRST_CONN_CFG_TAG));
  APP_ERROR_CHECK(sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV,
                                          adv_handle_, PRST_BLE_ADV_TX_POWER));
#if PRST_BLE_DEBUG
  NRF_LOG_INFO("[ble] Advertising started.\n");
#endif
}

void prst_adv_stop() {
  ret_code_t err_code;
  err_code = sd_ble_gap_adv_stop(adv_handle_);
  APP_ERROR_CHECK(err_code);
#if PRST_BLE_DEBUG
  NRF_LOG_INFO("[ble] Advertising stopped.\n");
#endif
}