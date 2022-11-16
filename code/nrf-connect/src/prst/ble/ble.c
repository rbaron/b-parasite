#include "ble.h"

#include <logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/settings/settings.h>

#include "prst/macros.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

int prst_ble_init() {
  RET_IF_ERR(bt_enable(/*bt_reader_cb_t=*/NULL));
  if (IS_ENABLED(CONFIG_SETTINGS)) {
    RET_IF_ERR_MSG(settings_load(), "Error in settings_load()");
  }
  return 0;
}

int prst_ble_adv_start() {
  return bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
                         ARRAY_SIZE(sd));
}

int prst_ble_adv_stop() {
  return bt_le_adv_stop();
}

int prst_ble_adv_set_data(const prst_sensors_t *sensors) {
  return 0;
}