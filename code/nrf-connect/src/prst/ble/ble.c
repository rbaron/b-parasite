#include "ble.h"

#include <logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/settings/settings.h>

#include "prst/ble/encoding.h"
#include "prst/macros.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_DBG);

static uint8_t service_data[PRST_BLE_ENCODING_SERVICE_DATA_LEN] = {0};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_SVC_DATA16, service_data, ARRAY_SIZE(service_data)),
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

// bt_addr_le_t.a holds the MAC address in big-endian.
static int get_mac_addr(bt_addr_le_t *out) {
  struct bt_le_oob oob;
  RET_IF_ERR(bt_le_oob_get_local(BT_ID_DEFAULT, &oob));
  LOG_HEXDUMP_DBG(oob.addr.a.val, ARRAY_SIZE(oob.addr.a.val),
                  "Read address using bt_le_oob_get_local");
  *out = oob.addr;

  // This API doesn't seem to work here.
  // static bt_addr_le_t addrs[CONFIG_BT_ID_MAX];
  // static size_t count;
  // bt_id_get(addrs, &count);
  // LOG_DBG("[bt_id_get] Received %d addresses", count);

  return 0;
}

int prst_ble_init() {
  RET_IF_ERR(bt_enable(/*bt_reader_cb_t=*/NULL));
  if (IS_ENABLED(CONFIG_SETTINGS)) {
    RET_IF_ERR_MSG(settings_load(), "Error in settings_load()");
  }

  struct bt_le_oob oob;
  RET_IF_ERR(bt_le_oob_get_local(BT_ID_DEFAULT, &oob));
  LOG_INF("ADDR: ");
  LOG_HEXDUMP_INF(oob.addr.a.val, 6, ":");
  return 0;
}

int prst_ble_adv_start() {
  // If BT_LE_ADV_NCONN_IDENTITY, this function will advertise with a static MAC
  // address programmed in the chip. If BT_LE_ADV_NCONN, this function returns
  // advertises with a random MAC each time.
  return bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad), sd,
                         ARRAY_SIZE(sd));
}

int prst_ble_adv_stop() {
  return bt_le_adv_stop();
}

int prst_ble_adv_set_data(const prst_sensors_t *sensors) {
  bt_addr_le_t addr;
  RET_IF_ERR(get_mac_addr(&addr));
  LOG_HEXDUMP_WRN(addr.a.val, ARRAY_SIZE(addr.a.val), "GOT ADDR: ");
  return prst_ble_encode_service_data(sensors, &addr, service_data,
                                      sizeof(service_data));
}