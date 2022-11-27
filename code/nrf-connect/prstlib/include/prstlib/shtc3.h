#ifndef _PRST_SHT3C_H_
#define _PRST_SHT3C_H_

// Values from the SHTC3 datasheet.
#define PRST_SHTC3_ADDR 0x70
#define PRST_SHTC3_CMD_SLEEP 0xb098
#define PRST_SHTC3_CMD_WAKEUP 0x3517
#define PRST_SHTC3_CMD_MEASURE_TFIRST_LOW_POWER 0x609c
#define PRST_SHTC3_CMD_MEASURE_TFIRST_NORMAL 0x7866

typedef struct {
  // Temperature in Celcius.
  float temp_c;
  // Relative humidity in [0, 1.0].
  float rel_humi;
} prst_shtc3_read_t;

int prst_shtc3_read(prst_shtc3_read_t *out);

#endif  // _PRST_SHT3C_H_