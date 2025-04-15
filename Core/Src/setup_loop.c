#include "setup_loop.h"

#include "imu.h"

uint32_t a = 0;
uint8_t tx_buffer[1000] = "Hello World!\n";

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

    ble_log(tx_buffer, 14);
}
inline void loop(void) {
    ble_log(tx_buffer, 14);
    delay_ms(1000);
}
