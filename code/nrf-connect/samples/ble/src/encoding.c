#include "encoding.h"

#include <prstlib/macros.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(ble, LOG_LEVEL_INF);

int prst_ble_encode_service_data(const prst_sensors_t* sensors,
                                 const bt_addr_le_t* bt_addr, uint8_t* out,
                                 uint8_t out_len) {
  RET_CHECK(out_len >= CONFIG_PRST_BLE_ENCODING_SERVICE_DATA_LEN,
            "Buffer is not large enough");

#if CONFIG_PRST_BLE_ENCODING_BPARASITE_V2
  // 0x181a - Environmental sensing service UUID.
  out[0] = 0x1a;
  out[1] = 0x18;
  // Four bits for the protocol version.
  out[2] |= (2 << 4) & 0xf0;
  // Bit 0 of byte 0 specifies whether or not ambient light data exists in the
  // payload.
#if DT_NODE_EXISTS(DT_NODELABEL(photo_transistor)) || DT_NODE_EXISTS(DT_NODELABEL(ldr))
  out[2] |= 1;
#endif
  // 4 bits for a small wrap-around counter for deduplicating messages on the
  // receiver.

  static uint8_t run_counter;

  out[3] = run_counter++ & 0x0f;
  out[4] = sensors->batt.adc_read.millivolts >> 8;
  out[5] = sensors->batt.adc_read.millivolts & 0xff;
  int16_t temp_centicelsius = 100 * sensors->shtc3.temp_c;
  out[6] = temp_centicelsius >> 8;
  out[7] = temp_centicelsius & 0xff;
  uint16_t humi = sensors->shtc3.rel_humi * UINT16_MAX;
  out[8] = humi >> 8;
  out[9] = humi & 0xff;
  uint16_t soil_moisture = sensors->soil.percentage * UINT16_MAX;
  out[10] = soil_moisture >> 8;
  out[11] = soil_moisture & 0xff;
  // MAC address in big-endian.
  memcpy(out + 12, bt_addr->a.val, BT_ADDR_SIZE);
  out[18] = sensors->photo.brightness >> 8;
  out[19] = sensors->photo.brightness & 0xff;

// https://bthome.io/v1/
#elif CONFIG_PRST_BLE_ENCODING_BTHOME_V1

  // 0x181c - User data service UUID.
  out[0] = 0x1c;
  out[1] = 0x18;

  // 1. Soil moisture.
  // uint16_t.
  out[2] = (0b000 << 5) | 3;
  // Type of measurement - Moisture.
  out[3] = 0x14;
  // Value. Factor of 0.01, so we need to multiply our the value in 100% by
  // 1/0.01 = 100.
  uint16_t soil_val = 10000 * sensors->soil.percentage;
  out[4] = soil_val & 0xff;
  out[5] = soil_val >> 8;
  // 2. Temp.
  // int16_t.
  out[6] = (0b001 << 5) | 3;
  // Type of measurement - temperature.
  out[7] = 0x02;
  // Value. Factor 0.01.
  int16_t temp_val = 100 * sensors->shtc3.temp_c;
  out[8] = temp_val & 0xff;
  out[9] = temp_val >> 8;
  // 3. Humidity
  // uint16_t.
  out[10] = (0b000 << 5) | 3;
  // Type - humidity.
  out[11] = 0x03;
  // Value. Factor 0.01, over 100%.
  uint16_t humi_val = 10000 * sensors->shtc3.rel_humi;
  out[12] = humi_val & 0xff;
  out[13] = humi_val >> 8;
  // 4. Battery voltage.
  // uint16_t.
  out[14] = (0b000 << 5) | 3;
  // Type - voltage.
  out[15] = 0x0c;
  // Value. Factor of 0.001.
  uint16_t batt_val = sensors->batt.adc_read.millivolts;
  out[16] = batt_val & 0xff;
  out[17] = batt_val >> 8;

// https://bthome.io/format/
#elif CONFIG_PRST_BLE_ENCODING_BTHOME_V2
  // 0xfcd2 - bthome.io service UUID.
  out[0] = 0xd2;
  out[1] = 0xfc;

  // Service header - no encryption, bt home v2.
  out[2] = 0x40;

  // Temperature.
  out[3] = 0x02;
  int16_t temp_val = 100 * sensors->shtc3.temp_c;
  out[4] = temp_val & 0xff;
  out[5] = temp_val >> 8;
  // Humidity.
  out[6] = 0x03;
  // Value. Factor 0.01, over 100%.
  uint16_t humi_val = 10000 * sensors->shtc3.rel_humi;
  out[7] = humi_val & 0xff;
  out[8] = humi_val >> 8;
  // Illuminance.
  out[9] = 0x05;
  // Value. Factor of 0.01.
  uint32_t lux_val = sensors->photo.brightness * 100;
  out[10] = lux_val & 0xff;
  out[11] = (lux_val >> 8) & 0xff;
  out[12] = (lux_val >> 16) & 0xff;
  // Battery voltage.
  out[13] = 0x0c;
  // Value. Factor of 0.001.
  uint16_t batt_val = sensors->batt.adc_read.millivolts;
  out[14] = batt_val & 0xff;
  out[15] = batt_val >> 8;
  // Soil moisture.
  out[16] = 0x14;
  // Factor of 0.01, so we need to multiply our the value in 100% by 1/0.01 = 100.
  uint16_t soil_val = 10000 * sensors->soil.percentage;
  out[17] = soil_val & 0xff;
  out[18] = soil_val >> 8;

#endif  // Encoding protocols

  LOG_HEXDUMP_DBG(out, out_len, "Encoded BLE adv: ");
  return 0;
}