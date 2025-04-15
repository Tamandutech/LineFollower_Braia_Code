#include "setup_loop.h"

#include <stdio.h>
#include <string.h>

uint32_t a = 0;
uint8_t tx_buffer[1000] = "Hello World!\n";

volatile uint32_t adc_buffer[18];

stmdev_ctx_t imu_ctx;
lsm6dsr_pin_int1_route_t int1_route;
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;

inline void setup(void) {
    imu_init(&imu_ctx, &int1_route);
    adc_start();

    ble_log(tx_buffer, 14);
}
inline void loop(void) {
    ble_log(tx_buffer, 14);

    float vref = 1.087f * 4095.0f / adc_buffer[0];
    float vbat = (adc_buffer[17] * vref / 4095.0f) * 5.6875f;  // 5.6875 é a constante do divisor de tensão
    snprintf((char *)tx_buffer, sizeof(tx_buffer),
             "Vbat [V]:%2.3f\tVref [V]:%2.3f\r\n", vbat, vref);
    ble_log(tx_buffer, strlen((char const *)tx_buffer));

    delay_ms(1000);
}
