#include "prstlib/shtc3.h"

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "prstlib/macros.h"

LOG_MODULE_REGISTER(shtc3, CONFIG_PRSTLIB_LOG_LEVEL);

static const struct i2c_dt_spec shtc3 = I2C_DT_SPEC_GET(DT_NODELABEL(shtc3));

static uint8_t buff[6];

static int write_cmd(uint16_t command) {
  static uint8_t cmd[2];
  cmd[0] = command >> 8;
  cmd[1] = command & 0xff;
  RET_IF_ERR(i2c_write_dt(&shtc3, cmd, sizeof(cmd)));
  return 0;
}

int prst_shtc3_read(prst_shtc3_read_t *out) {
  RET_IF_ERR_MSG(!device_is_ready(shtc3.bus), "SHTC3 is not ready");

  // Wake the sensor up.
  RET_IF_ERR(write_cmd(PRST_SHTC3_CMD_WAKEUP));
  k_msleep(1);

  // Request measurement.
  RET_IF_ERR(write_cmd(PRST_SHTC3_CMD_MEASURE_TFIRST_NORMAL));

  // Reading in normal (not low power) mode can take up to 12.1 ms, according to
  // the datasheet.
  k_msleep(20);

  // Read response.
  RET_IF_ERR(i2c_read_dt(&shtc3, buff, 6));

  // Put the sensor in sleep mode.
  RET_IF_ERR(write_cmd(PRST_SHTC3_CMD_SLEEP));

  // TODO: verify the CRC of the measurements. The function is described in the
  // datasheet.

  out->temp_c = -45 + 175 * ((float)((buff[0] << 8) | buff[1])) / (1 << 16);
  out->rel_humi = ((float)((buff[3] << 8) | buff[4])) / UINT16_MAX;

  LOG_DBG("Read temp: %f oC (%d)", out->temp_c, (int)out->temp_c);
  LOG_DBG("Read humi: %.0f %%", 100.0 * out->rel_humi);
  return 0;
}