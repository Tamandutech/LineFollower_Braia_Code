#include "loop.hpp"

#include <stdlib.h>

#include "logger.h"
// #include <math.h>
#include <stdio.h>
#include <string.h>

#include "WS2812Driver.h"
#include "main.h"
#include "platform_functions.h"

bool last_run = false;

uint8_t leu_direita = 0;
int32_t pos_direita = 0;

bool suc_ok = false;

char tx_buffer[1000] = "Hello World!\n";

uint32_t ultimo_ciclo = 0;
uint32_t ultimo_print = 0;

uint32_t encoder_values[2];

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

// typedef struct {
//     int *dados;
//     size_t tamanho;
//     size_t capacidade;
// } Lista;

// void init_lista(Lista *lista) {
//     lista->capacidade = 10;
//     lista->tamanho = 0;
//     lista->dados = malloc(lista->capacidade * sizeof(int));
// }

// void adicionar_lista(Lista *lista, int valor) {
//     if (lista->tamanho >= lista->capacidade) {
//         lista->capacidade *= 2;
//         lista->dados = realloc(lista->dados, lista->capacidade * sizeof(int));
//     }
//     lista->dados[lista->tamanho++] = valor;
// }

// int buscar_marcacao_proxima(Lista *lista, int valor) {
//     for (size_t i = 0; i < lista->tamanho; i++) {
//         if (abs(lista->dados[i] - valor) <= 50) {
//             return lista->dados[i];
//         }
//     }
//     return -1; // Nenhuma próxima
// }

// Lista marcacoes_mapeadas;

#define sensorCount 12
uint32_t gmaxSensorValues[sensorCount] =
    {
        3661,
        3473,
        3655,
        3523,
        3427,
        3508,
        3493,
        3515,
        3533,
        3481,
        3476,
        3646};

uint32_t gminSensorValues[sensorCount] =
    {
        208,
        199,
        199,
        199,
        194,
        198,
        197,
        197,
        196,
        198,
        199,
        207};

// uint32_t gmaxSensorValues[sensorCount] = {0};
// uint32_t gminSensorValues[sensorCount] = {0};

uint32_t calibratedSensors[sensorCount] = {0};
int8_t calibrationInitialized = 0;

void calibrateSensors(uint32_t* frontSensors) {
    uint32_t lmaxSensorValues[sensorCount];
    uint32_t lminSensorValues[sensorCount];

    // Primeira iteração ele seta os buffers
    if (!calibrationInitialized) {
        for (uint8_t i = 0; i < sensorCount; i++) {
            gmaxSensorValues[i] = 0;
            gminSensorValues[i] = 4095;
        }
        calibrationInitialized = 1;
    }

    // Itera sob as 10 primeiras leituras
    for (uint8_t j = 0; j < 10; j++) {
        update_adc(adc_buffer);

        // Itera sob cada sensor
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
        if (lmaxSensorValues[i] > gmaxSensorValues[i]) {
            gmaxSensorValues[i] = lmaxSensorValues[i];
        }

        // Update minimum only if the max of 10 readings was still lower than it
        // (we got 10 readings in a row lower than the existing minimum).
        if (lminSensorValues[i] < gminSensorValues[i]) {
            gminSensorValues[i] = lminSensorValues[i];
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
    bool onLine = false;
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
            onLine = true;
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
    // caso o valor desejado seja maior que a tensão da bateria, o valor desejado é a tensão da bateria
    //    float targetVoltage = desiredVoltage;
    //    if (targetVoltage > get_battery_voltage(adc_buffer)) {
    //        targetVoltage = get_battery_voltage(adc_buffer);
    //    }

    // map a value from 0v to vBat to 0 to 1000
    // float desiredSpeed = (targetVoltage * 1000.0f) / get_battery_voltage(adc_buffer);

    leftSpeed = desiredSpeed + PID;
    rightSpeed = desiredSpeed - PID;
    int MAX_SPEED = 1000;
    if (rightSpeed >= 0) {
        if (rightSpeed > MAX_SPEED) rightSpeed = MAX_SPEED;
        write_pin(motorDirDir, 0);

    } else {
        if (rightSpeed < -MAX_SPEED) rightSpeed = -MAX_SPEED;
        rightSpeed = (-1) * rightSpeed;
        write_pin(motorDirDir, 1);
    }

    if (leftSpeed >= 0) {
        if (leftSpeed > MAX_SPEED) leftSpeed = MAX_SPEED;
        write_pin(motorEsqDir, 0);

    } else {
        if (leftSpeed < -MAX_SPEED) leftSpeed = -MAX_SPEED;
        leftSpeed = (-1) * leftSpeed;
        write_pin(motorEsqDir, 1);
    }

    set_pwm(motorDirPWM, (uint16_t)rightSpeed);
    set_pwm(motorEsqPWM, (uint16_t)leftSpeed);
}

int lateral_count = 0;
void ler_laterais() {
    static uint8_t readingWhiteRight = 0;
    static uint8_t readingWhiteLeft = 0;
    static uint8_t readingIntersec = 0;
    static uint8_t firstTimeRight = 1;

    static int qtdLeftMark = 0;
    static int qtdRightMark = 0;

    update_adc(adc_buffer);

    if ((adc_buffer[0] < 2000 || adc_buffer[1] < 2000) || (adc_buffer[14] < 2000 || adc_buffer[15] < 2000)) {
        if ((adc_buffer[0] < 500 || adc_buffer[1] < 500) && (adc_buffer[14] > 3000 || adc_buffer[15] > 3000) && readingWhiteLeft == 0) {
            update_encoder_values(encoder_values);
            qtdLeftMark++;
            readingWhiteLeft = 1;
            bleLog("%02d Encoder: %f\n", lateral_count++, ((encoder_values[0] + encoder_values[1]) / 2.0f));
            // adicionar_lista(&marcacoes_mapeadas, leitura_atual);
        }

        else if ((adc_buffer[0] > 3000 || adc_buffer[1] > 3000) && (adc_buffer[14] < 2000 || adc_buffer[15] < 2000) && readingWhiteRight == 0 && firstTimeRight == 1) {
            update_encoder_values(encoder_values);
            reset_encoder_values();
            qtdRightMark++;
            readingWhiteRight = 1;
            firstTimeRight = 0;
            encoder_values[0] = 0;
            bleLog("Inicio Pista\n");
            ble_log(tx_buffer, strlen(tx_buffer));
        }

        else if ((adc_buffer[0] < 2000 || adc_buffer[1] < 2000) && (adc_buffer[14] < 2000 || adc_buffer[15] < 2000) && readingIntersec == 0) {
            readingIntersec = 1;
            readingWhiteRight = 1;
            readingWhiteLeft = 1;
            // snprintf(tx_buffer, sizeof(tx_buffer), "Intersec \n");
            // ble_log(tx_buffer, strlen(tx_buffer));
        }
    }

    else {
        readingIntersec = 0;
        readingWhiteRight = 0;
        readingWhiteLeft = 0;
    }
}

// void setIdealMapping()
// {
//     //toda vez que ler um lateral, compara com a lista de marcações geradas no mapeamento e se estiver dentro do range de X pulsos,
//     //seta os valores de encoder para o valor mais próximo da lista.
//     int marcacao_proxima = buscar_marcacao_proxima(&marcacoes_mapeadas, leitura_atual);
//     if (marcacao_proxima != -1) {
//         set_right_encoder_values(marcacao_proxima);
//         printf("Encoder ajustado para: %d\n", marcacao_proxima);
//     }
// }

void controla_suc(uint16_t target) {
    static uint16_t last_pwm;
    static uint32_t last_update;

    if (MILISECONDS - last_update >= 3) {
        suc_ok = false;
        if (target > last_pwm) {
            last_pwm++;
        } else if (target < last_pwm) {
            last_pwm--;
        } else {
            suc_ok = true;
        }

        set_pwm(motorSucPWM, last_pwm);
    }
}

#define MM_PER_COUNT 0.016873f  // 0.01687378866674205352689896348441 valor perfeito

float robotSpeed = 0;
int32_t leftPrev = 0, rightPrev = 0;
float previousTime = 0;

void robotSpeedTask() {
    float timerNow = MICROSECONDS;
    int32_t leftNow = get_left_encoder_position();
    int32_t rightNow = get_right_encoder_position();

    int32_t leftDist = leftNow - leftPrev;
    int32_t rightDist = rightNow - rightPrev;

    float elapsedTime = timerNow - previousTime;
    float elapsedTimeMS = elapsedTime / 1000;

    leftPrev = leftNow;
    rightPrev = rightNow;

    previousTime = timerNow;

    // velocidade em m/s
    robotSpeed = (((leftDist + rightDist) / 2.0f) * MM_PER_COUNT) / elapsedTimeMS;
    // bleLog("RobotSpeed %f", robotSpeed);
    // bleLog("ElapsedTime %f", elapsedTime);
}

float translacionalErrorBuffer[9];  // buffer to store the last 5 values of translationalError
int translacionalErrorIndex = 0;    // index to keep track of the current position in the buffer
float translacionalError = 0;
float P_Translacional = 0;
float D_Translacional = 0;
float I_Translacional = 0;
float PIDTranslacional = 0;
float lastTranslacionalError = 0;
float last_pwm, run_pwm;

float curva_acel(float pwm_goal) {
    if (pwm_goal > last_pwm) {
        run_pwm = pwm_goal;
    } else if (pwm_goal < last_pwm) {
        last_pwm -= 0.02f;
        run_pwm = pwm_goal;
    } else {
        run_pwm = pwm_goal;
    }
    return run_pwm;
}

void calcula_PID_translacional(float KpParam_Translacional,
                               float KdParam_Translacional,
                               float KiParam_Translacional,
                               float desiredSpeed) {
    translacionalError = curva_acel(desiredSpeed) - robotSpeed;
    P_Translacional = translacionalError;
    D_Translacional = translacionalError - lastTranslacionalError;

    // update the buffer and calculate the sum of the last 5 values
    translacionalErrorBuffer[translacionalErrorIndex] = translacionalError;
    translacionalErrorIndex = (translacionalErrorIndex + 1) % 9;
    float sum = 0;
    for (int i = 0; i < 9; i++) {
        sum += translacionalErrorBuffer[i];
    }
    I_Translacional = sum;

    PIDTranslacional = (KpParam_Translacional * P_Translacional) + (KdParam_Translacional * D_Translacional) + (KiParam_Translacional * I_Translacional);
    lastTranslacionalError = translacionalError;
}

int loop_old(void) {
    mcu_start();
    // init_lista(&marcacoes_mapeadas);

    rgb_color_t led[4];
    for (;;) {
        update_encoder_values(encoder_values);
        delay_ms(10);
    }

    imu_init(&imu_ctx, &int1_route);
    // bleLog("Initializing calibration");
    // for (uint8_t i = 0; i < 200; i++) {
    //     calibrateSensors(&adc_buffer[2]);
    //     delay_ms(40);
    // }

    // bleLog("Calibracao concluida\nAguardando start...\n");

    for (;;) {
        if (last_run && !run) {
            update_encoder_values(encoder_values);
            bleLog("Encoders: %d, %d\n0", encoder_values[0], encoder_values[1]);
            last_run = false;
        }

        if (!run) {
            set_pwm(motorDirPWM, 0);
            set_pwm(motorEsqPWM, 0);
            if (MICROSECONDS - ultimo_ciclo >= 1000000) {
                controla_suc(0);
            }
        }

        ler_laterais();

        // bleLog ("[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]",
        //         adc_buffer[2], adc_buffer[3], adc_buffer[4], adc_buffer[5],
        //         adc_buffer[6], adc_buffer[7], adc_buffer[8], adc_buffer[9],
        //         adc_buffer[10], adc_buffer[11], adc_buffer[12], adc_buffer[13]);

        if (MICROSECONDS - ultimo_ciclo >= 1000 && run) {
            controla_suc(999);  // 600  999
            // snprintf(tx_buffer, sizeof(tx_buffer), "encoderA: %d, encoderB: %d, vBat: %2.3f vRef: %2.3f\n", encoder_values[0], encoder_values[1], get_battery_voltage(adc_buffer), 1.21f * 4095.0f / adc_buffer[16]);

            // ble_log(tx_buffer, strlen(tx_buffer));
            // ultimo_print = MICROSECONDS;

            if (suc_ok) {
                PIDControl(0.12, 1.80);  // seguidor 0.1, 1.9  perseguidor 0.19, 1.9
                // motorControl(150);     // 270  350
                // write_pin(motorDirDir, 0);
                // write_pin(motorEsqDir, 0);
                // set_pwm(motorDirPWM, (uint16_t)500);
                // set_pwm(motorEsqPWM, (uint16_t)500);
                // robotSpeedTask();
                update_encoder_values(encoder_values);

                if (encoder_values[0] > 242000 && encoder_values[0] <= 311000) {  // Segmento 1
                    motorControl(250);
                    led[0] = LED_COLOR_GREEN;
                    led[1] = LED_COLOR_GREEN;
                    led[2] = LED_COLOR_GREEN;
                    led[3] = LED_COLOR_GREEN;
                    setLedsColor(led, 4);
                } else if (encoder_values[0] > 311000 && encoder_values[0] <= 440000) {
                    motorControl(145);
                    led[0] = LED_COLOR_RED;
                    led[1] = LED_COLOR_RED;
                    led[2] = LED_COLOR_RED;
                    led[3] = LED_COLOR_RED;
                    setLedsColor(led, 4);
                } else if (((encoder_values[0] + encoder_values[1]) / 2) > 1512000) {
                    motorControl(0);
                    led[0] = LED_COLOR_WHITE;
                    led[1] = LED_COLOR_WHITE;
                    led[2] = LED_COLOR_WHITE;
                    led[3] = LED_COLOR_WHITE;
                    setLedsColor(led, 4);
                } else {
                    motorControl(165);        // 300
                    led[0] = LED_COLOR_CYAN;  // Muda o LED para verde
                    led[1] = LED_COLOR_CYAN;
                    led[2] = LED_COLOR_CYAN;
                    led[3] = LED_COLOR_CYAN;
                    setLedsColor(led, 4);
                }
            }

            last_run = true;
            ultimo_ciclo = MICROSECONDS;
        }

        // if (MILISEONDS - ultimo_print >= 200 && !run) {
        //     arraySensorError();
        //     // snprintf(tx_buffer, sizeof(tx_buffer), "Sensor: %d  \n", arrayError);
        //     // snprintf(tx_buffer, sizeof(tx_buffer), "Velocidade motores: %.2f | %.2f \n", leftSpeed, rightSpeed);
        //     snprintf(tx_buffer, sizeof(tx_buffer), "sensors: %d\nsensor0 min: %d, sensor1 min: %d, sensor2 min: %d, sensor3 min: %d, sensor4 min: %d, sensor5 min: %d, sensor6 min: %d, sensor7 min: %d, sensor8 min: %d, sensor9 min: %d, sensor10 min: %d, sensor11 min: %d\n",
        //              lastPosition, gminSensorValues[0], gminSensorValues[1], gminSensorValues[2], gminSensorValues[3], gminSensorValues[4], gminSensorValues[5], gminSensorValues[6], gminSensorValues[7], gminSensorValues[8], gminSensorValues[9], gminSensorValues[10], gminSensorValues[11]);
        //     snprintf(tx_buffer, sizeof(tx_buffer), "sensors: %d\nsensor0 max: %d, sensor1 max: %d, sensor2 max: %d, sensor3 max: %d, sensor4 max: %d, sensor5 max: %d, sensor6 max: %d, sensor7 max: %d, sensor8 max: %d, sensor9 max: %d, sensor10 max: %d, sensor11 max: %d\n",
        //            lastPosition, gmaxSensorValues[0], gmaxSensorValues[1], gmaxSensorValues[2], gmaxSensorValues[3], gmaxSensorValues[4], gmaxSensorValues[5], gmaxSensorValues[6], gmaxSensorValues[7], gmaxSensorValues[8], gmaxSensorValues[9], gmaxSensorValues[10], gmaxSensorValues[11]);

        //     // update_encoder_values(encoder_values);
        //     // snprintf(tx_buffer, sizeof(tx_buffer), "encoderA: %d, encoderB: %d, vBat: %2.3f vRef: %2.3f\n", encoder_values[0], encoder_values[1], get_battery_voltage(adc_buffer), 1.21f * 4095.0f / adc_buffer[16]);

        //     ble_log(tx_buffer, strlen(tx_buffer));
        //     ultimo_print = MILISECONDS;
        // }
    }
}
