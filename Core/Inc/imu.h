#ifndef IMU_H
#define IMU_H

#include <stdint.h>

#include "lsm6dsr_reg.h"
#include "platform_functions.h"

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route);

#endif /* LSM6DSR_PLATFORM_INIT_H */
