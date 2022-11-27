#ifndef _PRST_BLE_ENCODING_H_
#define _PRST_BLE_ENCODING_H_

#include <prstlib/sensors.h>
#include <zephyr/bluetooth/bluetooth.h>

int prst_ble_encode_service_data(const prst_sensors_t* sensors,
                                 const bt_addr_le_t* bt_addr, uint8_t* out,
                                 uint8_t out_len);

#endif  // _PRST_BLE_ENCODING_H_