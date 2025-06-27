#include "platform_functions.h"

// definições específicas de hardware

#include "main.h"

volatile uint32_t adc1_buffer[9];
volatile uint32_t adc2_buffer[9];
volatile uint8_t rx_buffer[32] = {0};

// volatile uint32_t last_adc1_time = 0;
// volatile uint32_t last_adc2_time = 0;
// volatile uint32_t adc1_update_time = 0;
// volatile uint32_t adc2_update_time = 0;

// teste de tempo de atualização do adc // ultimo teste 276,6 us
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
//     uint32_t time = NANOSECONDS;  // Get the current time in milliseconds
//     if (hadc->Instance == ADC1) {
//         adc1_update_time = time - last_adc1_time;  // Calcula o tempo de atualização
//         last_adc1_time = time;                     // Atualiza o tempo da última conversão
//     } else if (hadc->Instance == ADC2) {
//         adc2_update_time = time - last_adc2_time;  // Calcula o tempo de atualização
//         last_adc2_time = time;                     // Atualiza o tempo da última conversão
//     }
//     adc_update_time = adc1_update_time + adc2_update_time;  // Atualiza o tempo total de atualização
// }

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // se o buffer conter "1" run deve ser = 0, se contiver "2" run deve ser = 1
        if (rx_buffer[0] == '1') {
            run = 0;
        } else if (rx_buffer[0] == '2') {
            reset_encoder_values();
            run = 1;
        } 

        start_ble_cmd_listening();  // Reinicia a recepção DMA
    }
}

// Definições comuns a serem usadas no main loop e demais arquivos interplataforma

pinhandler_t bootSw = {Boot_sw_GPIO_Port, Boot_sw_Pin};

pinhandler_t motorDirDir = {motor1dir_GPIO_Port, motor1dir_Pin};
pwmhandler_t motorDirPWM = {&htim8, TIM_CHANNEL_3};
pinhandler_t motorEsqDir = {motor2dir_GPIO_Port, motor2dir_Pin};
pwmhandler_t motorEsqPWM = {&htim8, TIM_CHANNEL_1};

pwmhandler_t motorSucPWM = {&htim5, TIM_CHANNEL_2};

void mcu_start(void) {
    ble_log("Hello World!\n", 14);

    // inicia timer
    HAL_TIM_Base_Start(&htim2);

    // inicia pwm
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);

    // inicia encoders
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    // inicia adc
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_Delay(100);
    HAL_ADC_Start_DMA(&hadc1, adc1_buffer, 9);
    HAL_ADC_Start_DMA(&hadc2, adc2_buffer, 9);
    HAL_Delay(50);
    uint32_t adc_buffer[18] = {0};
    uint8_t tx_buffer[40] = {0};
    snprintf(tx_buffer, sizeof(tx_buffer), "tensão da bateria: %2.2f\n", get_battery_voltage(adc_buffer));
    ble_log(tx_buffer, strlen(tx_buffer));
    HAL_Delay(50);

    start_ble_cmd_listening();  // Reinicia a recepção DMA
    ble_log("MCU Iniciado\n", 14);
}

void delay_ms(uint32_t millisec) {
    uint32_t tickstart = MILISEONDS;
    while ((MILISEONDS - tickstart) < millisec) {
        __NOP();  // No Operation
    }
}

void delay_us(uint32_t microsec) {
    uint32_t tickstart = MICROSECONDS;
    while ((MICROSECONDS - tickstart) < microsec) {
    }
}

void delay_ns(uint32_t nanosec) {
    uint32_t tickstart = NANOSECONDS;
    while ((NANOSECONDS - tickstart) < nanosec) {
    }
}

void ble_log(uint8_t *tx_buffer, uint16_t len) {
    HAL_UART_Transmit_DMA(&BLE_BUS, (const uint8_t *)tx_buffer, len);
}

void start_ble_cmd_listening(void) {
    HAL_UART_Receive_DMA(&BLE_BUS, rx_buffer, 1);  // Inicia a recepção DMA
}

uint8_t read_pin(pinhandler_t pin) {
    return (uint8_t)HAL_GPIO_ReadPin(pin.port, pin.pin);
}

void write_pin(pinhandler_t pin, uint8_t state) {
    if (state) {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(pin.port, pin.pin, GPIO_PIN_RESET);
    }
}

void toggle_pin(pinhandler_t pin) {
    HAL_GPIO_TogglePin(pin.port, pin.pin);
}

void set_pwm(pwmhandler_t pwmpin, uint16_t dutty) {
    __HAL_TIM_SET_COMPARE(pwmpin.htim, pwmpin.channel, dutty);
}

void reset_encoder_values() {
    __HAL_TIM_SET_COUNTER(&htim3, 0);  // Motor Esquerdo
    __HAL_TIM_SET_COUNTER(&htim4, 0);  // Motor Direito
}

void set_right_encoder_values(uint16_t rightEncoderValue){
    __HAL_TIM_SET_COUNTER(&htim4, rightEncoderValue);  // Motor Direito
}

void set_left_encoder_values(uint16_t leftEncoderValue){
    __HAL_TIM_SET_COUNTER(&htim3, leftEncoderValue);  // Motor Esquerdo
}

void update_encoder_value(int32_t *encoderArray) {
    encoderArray[0] = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);  // Motor Esquerdo
    encoderArray[1] = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);  // Motor Direito
}

void update_adc(uint32_t *adc_buffer) {
    // adc1
    adc_buffer[12] = adc1_buffer[0];
    adc_buffer[11] = adc1_buffer[1];
    adc_buffer[10] = adc1_buffer[2];
    adc_buffer[9] = adc1_buffer[3];
    adc_buffer[15] = adc1_buffer[4];
    adc_buffer[14] = adc1_buffer[5];
    adc_buffer[1] = adc1_buffer[6];
    adc_buffer[2] = adc1_buffer[7];
    adc_buffer[16] = adc1_buffer[8];  // Referência interterna

    // adc2
    adc_buffer[6] = adc2_buffer[0];
    adc_buffer[5] = adc2_buffer[1];
    adc_buffer[4] = adc2_buffer[2];
    adc_buffer[13] = adc2_buffer[3];
    adc_buffer[3] = adc2_buffer[4];
    adc_buffer[0] = adc2_buffer[5];
    adc_buffer[7] = adc2_buffer[6];
    adc_buffer[8] = adc2_buffer[7];
    adc_buffer[17] = adc2_buffer[8];  // Bateria
}

float get_battery_voltage(uint32_t *adc_buffer) {
    update_adc(adc_buffer);

    // calcula a tensão da bateria com base na leitura do ADC
    float vref = 1.21f * 4095.0f / adc_buffer[16];            // Vref = 1.2V, 4095 é o valor máximo do ADC
    float vbat = (adc_buffer[17] * vref / 4095.0f) * 6.015f;  // 5.6875 é a constante do divisor de tensão
    return vbat;                                              // Retorna a tensão da bateria
}

/** Please note that is MANDATORY: return 0 -> no Error.**/
int32_t write_imu(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Write(handle, LSM6DSR_I2C_ADD_H, reg,
                             I2C_MEMADD_SIZE_8BIT, (uint8_t *)bufp, len, 1000);
}
int32_t read_imu(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
    return HAL_I2C_Mem_Read(handle, LSM6DSR_I2C_ADD_H, reg,
                            I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);
}

void imu_init(stmdev_ctx_t *imu_ctx, lsm6dsr_pin_int1_route_t *int1_route) {
    uint8_t whoamI, rst;

    imu_ctx->write_reg = write_imu;
    imu_ctx->read_reg = read_imu;
    imu_ctx->mdelay = delay_ms;
    imu_ctx->handle = &IMU_BUS;

    while (1) {
        ble_log("Conectando com a IMU...\n", 25);
        lsm6dsr_device_id_get(imu_ctx, &whoamI);
        if (whoamI != LSM6DSR_ID) {
            ble_log("Error: Device ID mismatch\n", 27);
            delay_ms(500);
        } else {
            ble_log("IMU conectada\n", 15);
            break;
        }
    }

    lsm6dsr_reset_set(imu_ctx, PROPERTY_ENABLE);
    do {
        lsm6dsr_reset_get(imu_ctx, &rst);
    } while (rst);

    // INIT PARA POLLING
    /* Disable I3C interface */
    lsm6dsr_i3c_disable_set(imu_ctx, LSM6DSR_I3C_DISABLE);
    /* Enable Block Data Update */
    lsm6dsr_block_data_update_set(imu_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate */
    lsm6dsr_xl_data_rate_set(imu_ctx, LSM6DSR_XL_ODR_12Hz5);
    lsm6dsr_gy_data_rate_set(imu_ctx, LSM6DSR_GY_ODR_12Hz5);
    /* Set full scale */
    lsm6dsr_xl_full_scale_set(imu_ctx, LSM6DSR_2g);
    lsm6dsr_gy_full_scale_set(imu_ctx, LSM6DSR_2000dps);
    /* Configure filtering chain(No aux interface)
     * Accelerometer - LPF1 + LPF2 path
     */
    lsm6dsr_xl_hp_path_on_out_set(imu_ctx, LSM6DSR_LP_ODR_DIV_100);
    lsm6dsr_xl_filter_lp2_set(imu_ctx, PROPERTY_ENABLE);
}
