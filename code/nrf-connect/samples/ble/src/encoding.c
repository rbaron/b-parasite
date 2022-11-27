#include "encoding.h"

#include <logging/log.h>
#include <prstlib/macros.h>

LOG_MODULE_DECLARE(ble, LOG_LEVEL_DBG);

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
  out[2] |= 1;
  // 4 bits for a small wrap-around counter for deduplicating messages on the
  // receiver.
  // out[3] = sensors->run_counter & 0x0f;
  out[4] = sensors->batt.millivolts >> 8;
  out[5] = sensors->batt.millivolts & 0xff;
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
#elif CONFIG_PRST_BLE_ENCODING_BTHOME
  // TODO.
  memset(out, 0xab, out_len);
#endif  // Enccoding protocols

  return 0;
}