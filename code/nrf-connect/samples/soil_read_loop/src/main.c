#include <prstlib/macros.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

static const struct pwm_dt_spec soil_pwm_dt =
    PWM_DT_SPEC_GET(DT_NODELABEL(soil_pwm));
static const uint32_t pulse = DT_PROP(DT_NODELABEL(soil_pwm), pulse);

struct gpio_dt_spec fast_disch_dt =
    GPIO_DT_SPEC_GET(DT_NODELABEL(fast_disch), gpios);

static const struct adc_dt_spec adc_soil_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);

static const struct adc_dt_spec adc_batt_spec =
    ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 1);

static int16_t soil_buf;
static struct adc_sequence soil_sequence = {
    .buffer = &soil_buf,
    .buffer_size = sizeof(soil_buf),
};

static int16_t batt_buf;
static struct adc_sequence batt_sequence = {
    .buffer = &batt_buf,
    .buffer_size = sizeof(batt_buf),
};

int main(void) {
  RET_IF_ERR(!device_is_ready(fast_disch_dt.port));
  RET_IF_ERR(!device_is_ready(soil_pwm_dt.dev));

  // Configure analog-to-digital channels.
  RET_IF_ERR(adc_channel_setup_dt(&adc_soil_spec));
  RET_IF_ERR(adc_channel_setup_dt(&adc_batt_spec));

  // Configure fast discharge enable pin.
  RET_IF_ERR(gpio_pin_configure_dt(&fast_disch_dt, GPIO_OUTPUT));

  // Enable fast discharge circuit.
  RET_IF_ERR(gpio_pin_set_dt(&fast_disch_dt, 1));

  // Start PWM.
  RET_IF_ERR(pwm_set_dt(&soil_pwm_dt, soil_pwm_dt.period, pulse));

  RET_IF_ERR(adc_sequence_init_dt(&adc_soil_spec, &soil_sequence));
  RET_IF_ERR(adc_sequence_init_dt(&adc_batt_spec, &batt_sequence));

  LOG_INF("input_voltage;soil_adc_output");
  while (true) {
    k_msleep(500);

    RET_IF_ERR(adc_read(adc_batt_spec.dev, &batt_sequence));
    int32_t batt_val_mv = batt_buf;
    RET_IF_ERR(adc_raw_to_millivolts_dt(&adc_batt_spec, &batt_val_mv));

    RET_IF_ERR(adc_read(adc_soil_spec.dev, &soil_sequence));
    int32_t soil_val_mv = soil_buf;
    RET_IF_ERR(adc_raw_to_millivolts_dt(&adc_soil_spec, &soil_val_mv));

    LOG_INF("%.2f;%u", batt_val_mv / 1000.0f, soil_buf);
  }
}
