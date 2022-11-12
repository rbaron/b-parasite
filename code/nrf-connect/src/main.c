#include <drivers/i2c.h>
#include <logging/log.h>
#include <zephyr/zephyr.h>

#include "prst/shtc3.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

void main(void) {
  prst_shtc3_read_t shtc3_read = prst_shtc3_read();
}
