#include <logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/settings/settings.h>
#include <zephyr/zephyr.h>

#include "prst/adc.h"
#include "prst/button.h"
#include "prst/led.h"
#include "prst/macros.h"
#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME),
};

int main(void) {
  RET_IF_ERR(prst_adc_init());
  RET_IF_ERR(prst_led_init());
  RET_IF_ERR(prst_button_init());

  RET_IF_ERR(prst_led_flash(2));

  prst_adc_read_t batt;
  prst_adc_soil_moisture_t soil;
  prst_adc_photo_sensor_t photo;

  RET_IF_ERR(bt_enable(/*bt_reader_cb_t=*/NULL));

  if (IS_ENABLED(CONFIG_SETTINGS)) {
    RET_IF_ERR_MSG(settings_load(), "Error in settings_load()");
  }

  RET_IF_ERR(
      bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd)));

  while (true) {
    RET_IF_ERR(prst_adc_batt_read(&batt));
    RET_IF_ERR(prst_adc_soil_read(batt.voltage, &soil));
    RET_IF_ERR(prst_adc_photo_read(batt.voltage, &photo));

    LOG_INF("Batt: %d mV", batt.millivolts);
    LOG_INF("Soil: %.0f %% (%.3f mV)", 100 * soil.percentage,
            soil.adc_read.voltage);
    LOG_INF("Photo: %u lx (%.3f mV)", photo.brightness, soil.adc_read.voltage);
    k_msleep(500);
  }
}
