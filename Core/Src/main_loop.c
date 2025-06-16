#include "main_loop.h"

#include <stdio.h>
#include <string.h>

#include "WS2812Driver.h"

volatile uint8_t run = 0;
uint8_t last_run = 0;

uint8_t leu_direita = 0;
int32_t pos_direita = 0;

uint8_t suc_ok = 0;

uint8_t tx_buffer[1000] = "Hello World!\n";

uint32_t ultimo_ciclo = 0;
uint32_t ultimo_print = 0;

int32_t encoder_values[2];

// volatile uint32_t adc_update_time;
uint32_t adc_buffer[18];

stmdev_ctx_t imu_ctx;
lsm6dsr_pin_int1_route_t int1_route;
int16_t data_raw_acceleration[3];
int16_t data_raw_angular_rate[3];
int16_t data_raw_temperature;
float_t acceleration_mg[3];
float_t angular_rate_mdps[3];
float_t temperature_degC;

#define sensorCount 12
uint32_t gmaxSensorValues[sensorCount] = {0};
uint32_t gminSensorValues[sensorCount] = {0};
uint32_t calibratedSensors[sensorCount] = {0};
int8_t calibrationInitialized = 0;

void calibrateSensors(uint32_t* frontSensors) {
    uint32_t lmaxSensorValues[sensorCount];
    uint32_t lminSensorValues[sensorCount];

    if (!calibrationInitialized) {
        for (uint8_t i = 0; i < sensorCount; i++) {
            gmaxSensorValues[i] = 0;
            gminSensorValues[i] = 4095;
        }
        calibrationInitialized = 1;
    }

    for (uint8_t j = 0; j < 10; j++) {
        update_adc(adc_buffer);

        for (uint8_t i = 0; i < sensorCount; i++) {
            if ((j == 0) || frontSensors[i] > lmaxSensorValues[i])
                lmaxSensorValues[i] = frontSensors[i];

            if ((j == 0) || frontSensors[i] < lminSensorValues[i])
                lminSensorValues[i] = frontSensors[i];
        }
    }
    // record the min and max calibration values
    for (uint8_t i = 0; i < sensorCount; i++) {
        // Update maximum only if the min of 10 readings was still higher than it
        // (we got 10 readings in a row higher than the existing maximum).
        if (lminSensorValues[i] > gmaxSensorValues[i]) {
            gmaxSensorValues[i] = lminSensorValues[i];
        }

        // Update minimum only if the max of 10 readings was still lower than it
        // (we got 10 readings in a row lower than the existing minimum).
        if (lmaxSensorValues[i] < gminSensorValues[i]) {
            gminSensorValues[i] = lmaxSensorValues[i];
        }
    }
}

void readCalibrated(uint32_t* frontSensors) {
    update_adc(adc_buffer);
    for (uint8_t i = 0; i < sensorCount; i++) {
        uint32_t calmin, calmax;

        calmax = gmaxSensorValues[i];
        calmin = gminSensorValues[i];

        uint32_t denominator = calmax - calmin;
        int32_t value = 0;

        if (denominator != 0)
            value = (frontSensors[i] - calmin) * 1000 / denominator;

        if (value < 0)
            value = 0;
        else if (value > 1000)
            value = 1000;

        calibratedSensors[i] = value;
    }
}

uint32_t lastPosition = 0;
uint32_t readLine(uint32_t* sensorValues) {
    char onLine = 0;
    uint32_t avg = 0;  // this is for the weighted total
    uint16_t sum = 0;  // this is for the denominator, which is <= 64000

    readCalibrated(sensorValues);

    for (uint8_t i = 0; i < sensorCount; i++) {
        uint16_t value = calibratedSensors[i];
        // if (invertReadings)
        {
            value = 1000 - value;
        }

        // keep track of whether we see the line at all
        if (value > 200) {
            onLine = 1;
        }
        // only average in values that are above a noise threshold
        if (value > 50) {
            avg += (uint32_t)value * (i * 1000);
            sum += value;
        }
    }

    if (!onLine) {
        // If it last read to the left of center, return 0.
        if (lastPosition < (sensorCount - 1) * 1000 / 2) {
            // lastPosition = 0;
            return 0;
        }
        // If it last read to the right of center, return the max.
        else {
            // lastPosition = (sensorCount - 1) * 1000;
            return (sensorCount - 1) * 1000;
        }
    }

    lastPosition = avg / sum;
    return lastPosition;
}

int32_t arrayError;
float lastArrayError;
void arraySensorError() {
    arrayError = readLine(&adc_buffer[2]) - 5500;
}

float P;
float D;
float PID;
void PIDControl(float kp, float kd) {
    arraySensorError();
    P = arrayError;
    D = arrayError - lastArrayError;
    PID = (kp * P) + (kd * D);
    lastArrayError = arrayError;
}

float leftSpeed = 0;
float rightSpeed = 0;
void motorControl(float desiredVoltage) {
    // caso o valor desejado seja maior que a tensão da bateria, o valor desejado é a tensão da bateria
    float targetVoltage = desiredVoltage;
    if (targetVoltage > get_battery_voltage(adc_buffer)) {
        targetVoltage = get_battery_voltage(adc_buffer);
    }

    // map a value from 0v to vBat to 0 to 1000
    float desiredSpeed = (targetVoltage * 1000.0f) / get_battery_voltage(adc_buffer);

    leftSpeed = desiredSpeed + PID;
    rightSpeed = desiredSpeed - PID;
    int MAX_SPEED = 1000;
    if (rightSpeed >= 0) {
        if (rightSpeed > MAX_SPEED) rightSpeed = MAX_SPEED;
        write_pin(motorDirDir, 1);

    } else {
        if (rightSpeed < -MAX_SPEED) rightSpeed = -MAX_SPEED;
        rightSpeed = (-1) * rightSpeed;
        write_pin(motorDirDir, 0);
    }

    if (leftSpeed >= 0) {
        if (leftSpeed > MAX_SPEED) leftSpeed = MAX_SPEED;
        write_pin(motorEsqDir, 1);

    } else {
        if (leftSpeed < -MAX_SPEED) leftSpeed = -MAX_SPEED;
        leftSpeed = (-1) * leftSpeed;
        write_pin(motorEsqDir, 0);
    }

    set_pwm(motorDirPWM, (uint16_t)rightSpeed);
    set_pwm(motorEsqPWM, (uint16_t)leftSpeed);
}

void ler_laterais() {
    static uint32_t ultimo_tempo_esq;
    static uint32_t ultimo_tempo_dir;

    static uint8_t ultimo_esq;
    static uint8_t ultimo_dir;

    update_adc(adc_buffer);

    if (adc_buffer[0] < 600 || adc_buffer[1] < 600) {
        if (MILISEONDS - ultimo_tempo_esq >= 10 && !ultimo_esq) {
            update_encoder_value(encoder_values);
            // snprintf(tx_buffer, sizeof(tx_buffer), "%d, %d\n\0", encoder_values[0], encoder_values[1]);
            // ble_log(tx_buffer, strlen(tx_buffer));
            ultimo_tempo_esq = MILISEONDS;
        }
        ultimo_esq = 1;
    } else {
        ultimo_esq = 0;
    }

    if (adc_buffer[14] < 600 || adc_buffer[15] < 600) {
        if (MILISEONDS - ultimo_tempo_dir >= 10 && !ultimo_dir) {
            update_encoder_value(encoder_values);
            // snprintf(tx_buffer, sizeof(tx_buffer), "%d, %d\n\0", encoder_values[0], encoder_values[1]);
            // ble_log(tx_buffer, strlen(tx_buffer));

            if (encoder_values[1] > 5100) {
                leu_direita = 1;
                pos_direita = encoder_values[1];
            }

            ultimo_tempo_dir = MILISEONDS;
        }
        ultimo_dir = 1;
    } else {
        ultimo_dir = 0;
    }
}

void controla_suc(uint16_t target) {
    static uint16_t last_pwm;
    static uint32_t last_update;

    if (MILISEONDS - last_update >= 3) {
        suc_ok = 0;
        if (target > last_pwm) {
            last_pwm++;
        } else if (target < last_pwm) {
            last_pwm--;
        } else {
            suc_ok = 1;
        }

        set_pwm(motorSucPWM, last_pwm);
    }
}

void main_loop(void) {
    mcu_start();

    rgb_color_t led;
    for (;;) {
        led = (rgb_color_t){0, 0, 128};  // Inicializa o LED com azul
        setLedsColor(&led, 1);
        delay_ms(500);
        led = (rgb_color_t){0, 128, 0};  // Muda o LED para verde
        setLedsColor(&led, 1);
        delay_ms(500);
        led = (rgb_color_t){128, 0, 0};  // Muda o LED para vermelho
        setLedsColor(&led, 1);
        delay_ms(500);
    }

    imu_init(&imu_ctx, &int1_route);
    ble_log("iniciando calibracao\n", 22);
    for (uint8_t i = 0; i < 200; i++) {
        calibrateSensors(&adc_buffer[2]);
        delay_ms(40);
    }

    ble_log("Calibracao concluida\nAguardando start...\n", 42);

    for (;;) {
        if (last_run && !run) {
            update_encoder_value(encoder_values);
            snprintf(tx_buffer, sizeof(tx_buffer), "%d, %d\n\0", encoder_values[0], encoder_values[1]);
            ble_log(tx_buffer, strlen(tx_buffer));
            last_run = 0;
        }

        if (!run) {
            set_pwm(motorDirPWM, 0);
            set_pwm(motorEsqPWM, 0);
            if (MICROSECONDS - ultimo_ciclo >= 1000000) {
                controla_suc(0);
            }
        }

        ler_laterais();

        if (MICROSECONDS - ultimo_ciclo >= 1000 && run) {
            controla_suc(0);

            if (suc_ok) {
                PIDControl(0.1, 1.55);
                update_encoder_value(encoder_values);
                // motorControl(1.5f);

                if (encoder_values[1] < 400) {
                    motorControl(0.7f);
                } else if (encoder_values[1] < 700) {
                    motorControl(0.6f);
                } else if (encoder_values[1] < 5500) {
                    motorControl(0.7f);
                }

                if (encoder_values[1] > 5100 && leu_direita) {
                    motorControl(0.6f);
                    if (encoder_values[1] > pos_direita + 42) {
                        run = 0;
                    }
                }
            }

            last_run = 1;
            ultimo_ciclo = MICROSECONDS;
        }

        // if (MILISEONDS - ultimo_print >= 200 && !run) {
        //     snprintf(tx_buffer, sizeof(tx_buffer), "Sensor: %d  \n", arrayError);
        //     snprintf(tx_buffer, sizeof(tx_buffer), "Velocidade motores: %.2f | %.2f \n", leftSpeed, rightSpeed);
        //     snprintf(tx_buffer, sizeof(tx_buffer), "sensors: %d\nsensor0: %d, sendor1: %d, sensor2: %d, sensor3: %d, sensor4: %d, sensor5: %d, sensor6: %d, sensor7: %d, sensor8: %d, sensor9: %d, sensor10: %d, sensor11: %d\n",
        //              lastPosition, calibratedSensors[0], calibratedSensors[1], calibratedSensors[2], calibratedSensors[3], calibratedSensors[4], calibratedSensors[5],
        //              calibratedSensors[6], calibratedSensors[7], calibratedSensors[8], calibratedSensors[9], calibratedSensors[10], calibratedSensors[11]);

        //     update_encoder_value(encoder_values);
        //     snprintf(tx_buffer, sizeof(tx_buffer), "encoderA: %d, encoderB: %d, vBat: %2.3f vRef: %2.3f\n", encoder_values[0], encoder_values[1], get_battery_voltage(adc_buffer), 1.21f * 4095.0f / adc_buffer[16]);

        //     ble_log(tx_buffer, strlen(tx_buffer));
        //     ultimo_print = MILISEONDS;
        // }
    }
}
