#include "prst/ble.h"

#include <ble_advdata.h>
#include <ble_gap.h>
#include <nordic_common.h>
#include <nrf_delay.h>
#include <nrf_log.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble.h>

#include "prst_config.h"

// The connection to configure. We only have the one.
#define PRST_CONN_CFG_TAG 1

#define NON_CONNECTABLE_ADV_INTERVAL \
  MSEC_TO_UNITS(PRST_BLE_ADV_INTERVAL_IN_MS, UNIT_0_625_MS)

// Sensor data payload that will go into the advertisement message.
// We have a maximum of 20 bytes to play with here.
// Sensor data is encoded in unsigned 16 bits (2 bytes), and whenever multiple
// bytes are used to represent a single value, the encoding is big-endian:
/*
| Byte index |                          Description                            |
|------------|-----------------------------------------------------------------|
| 0          | Protocol version (4 bits) + reserved (3 bits) + has_lux* (1 bit)|
| 1          | Reserved (4 bits) + increasing, wrap-around counter (4 bits)    |
| 2-3        | Battery voltage in millivolts                                   |
| 4-5        | Temp in 1000  * Celsius  (protocol v1) or 100 * Celsius (v2)    |
| 6-7        | Relative air humidity, scaled from 0 (0%) to 0xffff (100%)      |
| 8-9        | Soil moisture, scaled from from 0 (0%) to 0xffff (100%)         |
| 10-15      | b-parasite's own MAC address                                    |
| 16-17*     | Ambient light in lux                                            |

* If the has_lux bit is set, bytes 16-17 shall contain the ambient light in lux.
If the has_lux bit is not set, bytes 16-17 may not exist or may contain
meaningless data. The reasons for this behavior are:
1. b-parasite version 1.0.0 has no light sensor and its advertisement data may
have only 16 bytes if its using an older firmware. In this case, has_lux shall
never be set;
2. b-parasite version 1.1.0 has space for an optional LDR. Users can configure
whether or not they have added the LDR by setting the PRST_HAS_LDR to 1 in
prst_config.h.
*/

#if PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BPARASITE_V2
#define SERVICE_UUID 0x181a
#define SERVICE_DATA_LEN 18
#elif PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BTHOME
#define SERVICE_UUID 0x181c
#define SERVICE_DATA_LEN 21
#else
#error "PRST_BLE_PROTOCOL is not properly configured"
#endif

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
#if PRST_BLE_EXPERIMENTAL_LONG_RANGE
    .flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE,
#else
    .flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED,
#endif
    .p_service_data_array = &advdata_service_data_,
    .service_data_count = 1,
};

// NRF supports multiple advertisement sets. This initialization is a request
// for configuring a new one.
static uint8_t adv_handle_ = BLE_GAP_ADV_SET_HANDLE_NOT_SET;

// Advertisement parameters.
static ble_gap_adv_params_t adv_params_;

// Stores the MAC address & type.
static ble_gap_addr_t gap_addr_ = {.addr_type =
                                       BLE_GAP_ADDR_TYPE_RANDOM_STATIC};

static void init_advertisement_data() {
  // We'll just broadcast our data, so we disallow connections and scan
  // requests.
#if PRST_BLE_EXPERIMENTAL_LONG_RANGE
  adv_params_.properties.type =
      BLE_GAP_ADV_TYPE_EXTENDED_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
  adv_params_.primary_phy = BLE_GAP_PHY_CODED;
  adv_params_.secondary_phy = BLE_GAP_PHY_CODED;
#else
  adv_params_.properties.type =
      BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
#endif

  adv_params_.p_peer_addr = NULL;
  adv_params_.filter_policy = BLE_GAP_ADV_FP_ANY;
  adv_params_.interval = NON_CONNECTABLE_ADV_INTERVAL;
  adv_params_.duration = 0;  // Never time out.

  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*)PRST_BLE_ADV_NAME,
                             strlen(PRST_BLE_ADV_NAME));

  uint32_t err_code =
      sd_ble_gap_adv_set_configure(&adv_handle_, &gap_adv_data_, &adv_params_);
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

#ifdef PRST_BLE_MAC_ADDR
  // Parses configured MAC address from PRST_BLE_MAC_ADDR.
  int mac_bytes[6];
  sscanf(PRST_BLE_MAC_ADDR, "%x:%x:%x:%x:%x:%x", &mac_bytes[0], &mac_bytes[1],
         &mac_bytes[2], &mac_bytes[3], &mac_bytes[4], &mac_bytes[5]);
  for (int i = 0; i < 6; i++) {
    gap_addr_.addr[5 - i] = (uint8_t)mac_bytes[i];
  }
  APP_ERROR_CHECK(sd_ble_gap_addr_set(&gap_addr_));
#endif

  APP_ERROR_CHECK(sd_ble_gap_addr_get(&gap_addr_));
  NRF_LOG_INFO("[ble] MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               gap_addr_.addr[5], gap_addr_.addr[4], gap_addr_.addr[3],
               gap_addr_.addr[2], gap_addr_.addr[1], gap_addr_.addr[0]);

  init_advertisement_data();
}

#if PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BPARASITE_V2
static void set_service_data_bparasite_protocol(
    const prst_sensor_data_t* sensors) {
  // Four bits for the protocol version.
  service_data[0] |= (2 << 4) & 0xf0;

  // Bit 0 of byte 0 specifies whether or not ambient light data exists in the
  // payload.
#if PRST_HAS_LDR || PRST_HAS_PHOTOTRANSISTOR
  service_data[0] |= 1;
#endif

  // 4 bits for a small wrap-around counter for deduplicating messages on the
  // receiver.
  service_data[1] = sensors->run_counter & 0x0f;

  service_data[2] = sensors->batt_mv >> 8;
  service_data[3] = sensors->batt_mv & 0xff;

  int16_t temp_centicelsius = 100 * sensors->temp_c;
  service_data[4] = temp_centicelsius >> 8;
  service_data[5] = temp_centicelsius & 0xff;

  service_data[6] = sensors->humi >> 8;
  service_data[7] = sensors->humi & 0xff;

  service_data[8] = sensors->soil_moisture >> 8;
  service_data[9] = sensors->soil_moisture & 0xff;

  // Bytes 10-15 (inclusive) contain the whole MAC address in big-endian.
  for (int i = 0; i < 6; i++) {
    service_data[10 + i] = gap_addr_.addr[5 - i];
  }

#if PRST_HAS_LDR || PRST_HAS_PHOTOTRANSISTOR
  service_data[16] = sensors->lux >> 8;
  service_data[17] = sensors->lux & 0xff;
#endif
}
#endif  // PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BPARASITE_V2

#if PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BTHOME
static void set_service_data_bthome_protocol(
    const prst_sensor_data_t* sensors) {
  // See values in https://bthome.io/.

  // 1. Soil moisture.
  // uint16_t.
  service_data[0] = (0b000 << 5) | 2;
  // Type of measurement. Temporarily using humidity.
  service_data[1] = 0x03;
  // Value. Factor of 0.01, so we need to multiply our the value in 100% by
  // 1/0.01 = 100.
  uint16_t soil_val = (10000 * sensors->soil_moisture) / UINT16_MAX;
  service_data[2] = soil_val & 0xff;
  service_data[3] = soil_val >> 8;

  // 2. Temp.
  // int16_t.
  service_data[4] = (0b001 << 5) | 2;
  // Type of measurement - temperature.
  service_data[5] = 0x02;
  // Value. Factor 0.01.
  int16_t temp_val = 100 * sensors->temp_c;
  service_data[6] = temp_val & 0xff;
  service_data[7] = temp_val >> 8;

  // 3. Humidity
  // uint16_t.
  service_data[8] = (0b000 << 5) | 2;
  // Type - humidity.
  service_data[9] = 0x03;
  // Value. Factor 0.01.
  uint16_t humi_val = (100 * sensors->humi) / UINT16_MAX;
  service_data[10] = humi_val & 0xff;
  service_data[11] = humi_val >> 8;

  // 4. Battery voltage.
  // uint16_t.
  service_data[12] = (0b000 << 5) | 2;
  // Type - voltage.
  service_data[13] = 0x0c;
  // Value. Factor of 0.001.
  uint16_t batt_val = sensors->batt_mv;
  service_data[14] = batt_val & 0xff;
  service_data[15] = batt_val >> 8;

  // 5. Illuminance
  // uint24_t.
  service_data[16] = (0b000 << 5) | 2;
  // Type - illuminance.
  service_data[17] = 0x05;
  // Value. Factor 0.01.
  uint32_t illu_value = 100 * sensors->lux;
  service_data[18] = illu_value & 0xff;
  service_data[19] = (illu_value >> 8) & 0xff;
  service_data[20] = (illu_value >> 16) & 0xff;
}
#endif  // PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BTHOME

void prst_ble_update_adv_data(const prst_sensor_data_t* sensors) {
#if PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BPARASITE_V2
  set_service_data_bparasite_protocol(sensors);
#elif PRST_BLE_PROTOCOL == PRST_BLE_PROTOCOL_BTHOME
  set_service_data_bthome_protocol(sensors);
#endif

  // Encodes adv_data_ into .gap_adv_data_.
  uint32_t err_code = ble_advdata_encode(
      &adv_data_, gap_adv_data_.adv_data.p_data, &gap_adv_data_.adv_data.len);
  APP_ERROR_CHECK(err_code);

#if PRST_BLE_DEBUG
  NRF_LOG_INFO("[ble] Encoded BLE adv packet:");
  for (int i = 0; i < sizeof(encoded_adv_data_); i++) {
    NRF_LOG_INFO("[ble] byte %02d: 0x%02x", i, encoded_adv_data_[i]);
    nrf_delay_ms(50);
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