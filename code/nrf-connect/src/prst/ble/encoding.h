#ifndef _PRST_BLE_ENCODING_H_
#define _PRST_BLE_ENCODING_H_

#include <zephyr/bluetooth/bluetooth.h>

#include "prst/data.h"

#define PRST_BLE_ENCODING_SERVICE_DATA_LEN 20

int prst_ble_encode_service_data(const prst_sensors_t* sensors,
                                 const bt_addr_le_t* bt_addr, uint8_t* out,
                                 uint8_t out_len);

#endif  // _PRST_BLE_ENCODING_H_