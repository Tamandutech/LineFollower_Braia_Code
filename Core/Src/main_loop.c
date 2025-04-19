#include "main_loop.h"

#include <stdio.h>
#include <string.h>

uint32_t a = 0;
uint8_t tx_buffer[1000] = "Hello World!\n";

uint32_t ultimo_ciclo = 0;

int32_t encoder_values[2];

// volatile uint32_t adc_update_time;
uint32_t adc_buffer[18];

stmdev_ctx_t imu_ctx;
lsm6dsr_pin_int1_route_t int1_route;
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;

#define sensorCount 12
uint32_t maxSensorValues[sensorCount] = {0};
uint32_t minSensorValues[sensorCount] = {0};
uint32_t calibratedSensors[sensorCount] = {0};

void calibrateSensors(uint32_t* frontSensors) {
    for (uint8_t j = 0; j < 10; j++) {
        update_adc(adc_buffer);

        for (uint8_t i = 0; i < sensorCount; i++) {
            if (frontSensors[i] > maxSensorValues[i])
                maxSensorValues[i] = frontSensors[i];

            if (frontSensors[i] < minSensorValues[i])
                minSensorValues[i] = frontSensors[i];
        }
    }
}

void readCalibrated(uint32_t* frontSensors) {
    update_adc(adc_buffer);
    for (uint8_t i = 0; i < sensorCount; i++) {
        uint32_t calmin, calmax;

        calmax = maxSensorValues[i];
        calmin = minSensorValues[i];

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
        // {
        value = 1000 - value;
        // }

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
void motorControl(float desiredSpeed) {
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

    set_pwm(motorDirPWM, rightSpeed);
    set_pwm(motorEsqPWM, leftSpeed);
}

void main_loop(void) {
    mcu_start();

    imu_init(&imu_ctx, &int1_route);
    ble_log("iniciando calibracao\n", 22);
    for (uint8_t i = 0; i < 200; i++) {
        calibrateSensors(&adc_buffer[2]);
        delay_ms(40);
    }
    ble_log("Calibracao concluida\n", 22);

    for (;;) {
        if (MICROSECONDS - ultimo_ciclo >= 1000) {
            readLine(&adc_buffer[2]);
            char sensorData[500] = {0};
            PIDControl(0.0423, 0);
            motorControl(70);
            update_encoder_value(encoder_values);

            // float vref = 1.2f * 4095.0f / adc_buffer[16];
            // float vbat = (adc_buffer[17] * vref / 4095.0f) * 5.6875f;  // 5.6875 é a constante do divisor de tensão

            /*for (int i = 0; i < sensorCount; i++) {
                static char string_buffer[30] = {0};
                sprintf((char*)string_buffer, "Sensor %d: %d ", i, calibratedSensors[i]);
                strcat((char*)sensorData, (char*)string_buffer);
            }
            strcat((char*)sensorData, "\n");*/
            // sprintf((char*)sensorData, "Velocidade motores: %.2f | %.2f \n", leftSpeed, rightSpeed);
            sprintf((char*)sensorData, "Sensor: %d  \n", arrayError);
            ble_log(sensorData, strlen((char*)sensorData));

            // set_pwm(motorDirPWM, 0);
            // set_pwm(motorEsqPWM, 0);
            // set_pwm(motorSucPWM, 0);

            ultimo_ciclo = MICROSECONDS;
        }
    }
}
