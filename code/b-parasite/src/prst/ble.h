#ifndef _PRST_BLE_H_
#define _PRST_BLE_H_

#include <app_error.h>
#include <stdint.h>

#include "prst/data.h"

// Initializes SoftDevice.
void prst_ble_init();

void prst_adv_start();

void prst_adv_stop();

void prst_ble_update_adv_data(const prst_sensor_data_t* sensors);

#endif  // _PRST_BLE_H_