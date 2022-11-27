#ifndef _PRST_BLE_BLE_H_
#define _PRST_BLE_BLE_H_

#include <prstlib/sensors.h>

int prst_ble_init();
int prst_ble_adv_start();
int prst_ble_adv_stop();
int prst_ble_adv_set_data(const prst_sensors_t *sensors);

#endif  // _PRST_BLE_BLE_H_