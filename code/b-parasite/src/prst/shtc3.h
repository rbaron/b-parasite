#ifndef _PRST_SHT3C_H_
#define _PRST_SHT3C_H_

typedef struct prst_shtc3_values {
  double temp_c;
  double humidity;
} prst_shtc3_read_t;

void prst_shtc3_init();
prst_shtc3_read_t prst_shtc3_read();

#endif  // _PRST_SHT3C_H_