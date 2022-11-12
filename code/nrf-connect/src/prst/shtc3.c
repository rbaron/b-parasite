#include "shtc3.h"

#include <drivers/i2c.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

LOG_MODULE_REGISTER(shtc3, LOG_LEVEL_DBG);

static const struct i2c_dt_spec shtc3 = I2C_DT_SPEC_GET(DT_NODELABEL(shtc3));

static uint8_t buff[6];

static void write_cmd(uint16_t command) {
  static uint8_t cmd[2];
  cmd[0] = command >> 8;
  cmd[1] = command & 0xff;
  if (i2c_write_dt(&shtc3, cmd, sizeof(cmd)) != 0) {
    LOG_ERR("Error writing command");
  }
}

prst_shtc3_read_t prst_shtc3_read() {
  // Wake the sensor up.
  write_cmd(PRST_SHTC3_CMD_WAKEUP);
  k_msleep(1);

  // Request measurement.
  write_cmd(PRST_SHTC3_CMD_MEASURE_TFIRST_NORMAL);

  // Reading in normal (not low power) mode can take up to 12.1 ms, according to
  // the datasheet.
  k_msleep(15);

  while (i2c_read_dt(&shtc3, buff, 6) != 0) {
    k_msleep(10);
  }

  // Put the sensor in sleep mode.
  write_cmd(PRST_SHTC3_CMD_SLEEP);

  // TODO: Uninit i2c to save power?

  // TODO: verify the CRC of the measurements. The function is described in the
  // datasheet.

  float temp_c = -45 + 175 * ((float)((buff[0] << 8) | buff[1])) / (1 << 16);
  float humi = ((float)((buff[3] << 8) | buff[4])) / UINT16_MAX;

  prst_shtc3_read_t ret = {.temp_c = temp_c, .rel_humi = humi};

  LOG_INF("Read temp: %f oC (%d)", ret.temp_c, (int)temp_c);
  LOG_INF("Read humi: %.0f %%", 100.0 * ret.rel_humi);
  return ret;
}