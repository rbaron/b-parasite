#ifndef _PRST_BLE_H_
#define _PRST_BLE_H_

#include <app_error.h>
#include <stdint.h>

// Initializes SoftDevice.
void prst_ble_init();

void prst_adv_start();

void prst_adv_stop();

void prst_ble_update_adv_data(uint16_t batt_millivolts);

#endif  // _PRST_BLE_H_