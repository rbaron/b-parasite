#ifndef _PRST_BLE_H_
#define _PRST_BLE_H_

#include <app_error.h>
#include <stdint.h>

// Initializes SoftDevice.
void prst_ble_init();

void prst_adv_start();

void prst_adv_stop();

void prst_ble_update_adv_data(uint16_t batt_millivolts,
                              uint16_t temp_millicelcius, uint16_t humidity,
                              uint16_t soil_moisture);

#endif  // _PRST_BLE_H_