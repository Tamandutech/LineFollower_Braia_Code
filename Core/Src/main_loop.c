#include "main_loop.h"

#include <stdio.h>
#include <string.h>

uint32_t a = 0;
uint8_t tx_buffer[1000] = "Hello World!\n";

volatile uint32_t adc_update_time;
volatile uint32_t adc_buffer[18];

stmdev_ctx_t imu_ctx;
lsm6dsr_pin_int1_route_t int1_route;
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;

void main_loop(void) {
    ble_log(tx_buffer, 14);
    imu_init(&imu_ctx, &int1_route);
    adc_start();
    pwm_start();

    for (;;) {
        update_adc();

        // Limpa o buffer
        tx_buffer[0] = '\0';

        // Adiciona ADC update time
        snprintf((char *)tx_buffer + strlen((char *)tx_buffer), sizeof(tx_buffer) - strlen((char *)tx_buffer),
                 "ADC update time: %lu\n", adc_update_time);

        float vref = 1.2f * 4095.0f / adc_buffer[16];
        float vbat = (adc_buffer[17] * vref / 4095.0f) * 5.6875f;  // 5.6875 é a constante do divisor de tensão

        // Adiciona Vbat e Vref
        snprintf((char *)tx_buffer + strlen((char *)tx_buffer), sizeof(tx_buffer) - strlen((char *)tx_buffer),
                 "Vbat [V]:%2.3f\tVref [V]:%2.3f\n",
                 vbat, vref);

        // Adiciona valores dos sensores
        snprintf((char *)tx_buffer + strlen((char *)tx_buffer), sizeof(tx_buffer) - strlen((char *)tx_buffer),
                 "S0: %d\tS1: %d\tS2: %d\tS3: %d\tS4: %d\tS5: %d\tS6: %d\tS7: %d\n"
                 "S8: %d\tS9: %d\tS10: %d\tS11: %d\tS12: %d\tS13: %d\tS14: %d\tS15: %d\n",
                 adc_buffer[0], adc_buffer[1], adc_buffer[2], adc_buffer[3], adc_buffer[4], adc_buffer[5], adc_buffer[6], adc_buffer[7],
                 adc_buffer[8], adc_buffer[9], adc_buffer[10], adc_buffer[11], adc_buffer[12], adc_buffer[13], adc_buffer[14], adc_buffer[15]);

        // PWM
        static uint16_t k = 1000;
        if (k < 300) {
            k++;
        } else {
            k = 0;
        }
        set_pwm(motorDirPWM, k);
        set_pwm(motorEsqPWM, k);
        set_pwm(motorSucPWM, k);

        // Adiciona valor do PWM
        snprintf((char *)tx_buffer + strlen((char *)tx_buffer), sizeof(tx_buffer) - strlen((char *)tx_buffer),
                 "PWM: %d\n", k);

        // Envia tudo de uma vez
        ble_log(tx_buffer, strlen((char const *)tx_buffer));

        delay_ms(300);
    }
}
