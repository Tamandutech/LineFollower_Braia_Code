#include "stdbool.h"

#include "Arduino.h"
#include <WiFi.h>
#include "SPIFFS.h"
#include <ESP8266FtpServer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"

#define LINE LINE_BLACK
#define TRACK_LEFT_MARKS 57
#define TRACK_RIGHT_MARKS 12

#define LINE_WHITE < 2000
#define LINE_BLACK < 2000

const char *ssid = "RFREITAS";
const char *password = "941138872";

extern "C"
{
  void app_main(void);
}

struct valuesS
{
  uint16_t *channel;
  double line;
};

struct valuesEnc
{
  int32_t encDir = 0;
  int32_t encEsq = 0;
  uint32_t media = 0;
};

struct valuesMarks
{
  int leftPassed = 0;
  int rightPassed = 0;
  bool sLatEsq;
  bool sLatDir;
};

struct valuesSamples
{
  valuesEnc motEncs;
  valuesS sLat;
  valuesS sArray;
  unsigned long time;
};

struct paramSpeed
{
  int max = 80;
  int min = 5;

  int rightBase = 40;
  int leftBase = 40;
  double rightActual = 0;
  double leftActual = 0;
};

struct valuesPID
{
  // Parâmetros
  double *input = NULL;
  double setpoint = 3500;
  double output = 0;
  double outputMin = -100;
  double outputMax = +100;
  double Kp = 0.01;
  double Ki = 0.00;
  double Kd = 0.10;

  // Variaveis de calculo
  float P = 0, I = 0, D = 0;
  float last_proportional = 0;
  float proportional = 0;
  float derivative = 0;
  float integral = 0;
};

struct valuesCar
{
  int state = 1; // 0: parado, 1: linha, 2: curva
  paramSpeed speed;
  valuesSamples *marksTrack;
  valuesMarks latMarks;
  valuesEnc motEncs;
  valuesS sLat;
  valuesS sArray;
};

TaskHandle_t xTaskMotors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskFTP;

FtpServer ftpSrv; //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

////////////////START FUNÇÕES////////////////

void calibSensor(QTRSensors *sensor)
{
  for (uint16_t i = 0; i < 200; i++)
  {
    sensor->calibrate();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void GetData(QTRSensors *array, QTRSensors *s_laterais, ESP32Encoder *enc_dir, ESP32Encoder *enc_esq, valuesCar *carVal)
{
  // Update erro do array
  carVal->sArray.line = array->readLineWhite(carVal->sArray.channel);

  // Update dos sensores laterais
  s_laterais->readCalibrated(carVal->sLat.channel);
  carVal->sLat.line = carVal->sLat.channel[0] - carVal->sLat.channel[1];

  // Update dos encoders dos motores
  carVal->motEncs.encDir = enc_dir->getCount() * -1;
  carVal->motEncs.encEsq = enc_esq->getCount();
  carVal->motEncs.media = (carVal->motEncs.encDir + carVal->motEncs.encEsq) / 2;
}

void SerialSend()
{

  /* Serial.printf("Setpoint: %.lf\n", dirPIDVal.setpoint);
  Serial.printf("Input: %.lf\n", dirPIDVal.input);
  Serial.printf("Output: %.lf\n", dirPIDVal.output);
  Serial.printf("---------\n");

  Serial.printf("LeftMotor: %d\n", MotorControl.getMotorSpeed(0));
  Serial.printf("RightMotor: %d\n", MotorControl.getMotorSpeed(1));
  Serial.printf("---------\n");

  Serial.printf("velMot\tvelPID\n");
  Serial.printf("%d\t%.lf\n", carVal.rightBaseSpeed, dirPIDVal.outputMax);
  Serial.printf("---------\n");

  Serial.printf("Kp\tKi\tKd\n");
  Serial.printf("%lf\t%lf\t%lf\n", dirPIDVal.Kp, dirPIDVal.Ki, dirPIDVal.Kd);
  Serial.printf("---------\n");

  Serial.printf("Ciclo: %ld\n", totalTime);
  Serial.printf("---------\n");

  Serial.printf("SL1: %d\n", sensorLat.sensorL1);
  Serial.printf("SL2: %d\n", sensorLat.sensorL2);
  Serial.printf("SL3: %d\n", sensorLat.sensorL3);
  Serial.printf("SL4: %d\n", sensorLat.sensorL4);
  Serial.printf("---------\n");

  Serial.printf("EncEsq: %d\n", motEncs.encEsq);
  Serial.printf("EncDir: %d\n", motEncs.encDir);
  Serial.printf("---------\n");

  Serial.printf("EncEsqCount: %d\n", carVal.leftMarksPassed);
  Serial.printf("EncDirCount: %d\n", carVal.rightMarksPassed);
  Serial.printf("---------\n"); */
}

void MotorControlFunc(ESP32MotorControl *MotorControl, valuesCar *carVal)
{
  if (carVal->state != 0)
  {
    MotorControl->motorForward(0);
    MotorControl->motorForward(1);
    MotorControl->motorSpeed(1, constrain(carVal->speed.leftActual, carVal->speed.min, carVal->speed.max));
    MotorControl->motorSpeed(0, constrain(carVal->speed.rightActual, carVal->speed.min, carVal->speed.max));
  }
  else
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    MotorControl->motorsStop();
  }
}

void processSLat(valuesCar *carVal)
{
  // Verifica se apenas um dos sensores leu a linha (evita interseções)
  if (carVal->sLat.line < -250 || carVal->sLat.line > 250)
  {
    if (carVal->sLat.line < -250)
    {
      carVal->latMarks.sLatEsq = true;
      carVal->latMarks.sLatDir = false;
    }
    else
    {
      carVal->latMarks.sLatEsq = false;
      carVal->latMarks.sLatDir = true;
    }
  }
}

void verifyState(valuesCar *carVal)
{
  if (carVal->latMarks.rightPassed < TRACK_RIGHT_MARKS /* || carVal.leftMarksPassed < TRACK_LEFT_MARKS */)
    if (carVal->latMarks.leftPassed % 2 != 0)
      carVal->state = 2;
    else
      carVal->state = 1;
  else
    carVal->state = 0;
}

void PIDFollow(valuesPID *PIDVal, valuesCar *carVal)
{
  // Calculo de valor lido
  PIDVal->proportional = (*PIDVal->input) - 3500;
  PIDVal->derivative = PIDVal->proportional - PIDVal->last_proportional;
  PIDVal->integral = PIDVal->integral + PIDVal->proportional;
  PIDVal->last_proportional = PIDVal->proportional;

  PIDVal->P = PIDVal->proportional * PIDVal->Kp;
  PIDVal->D = PIDVal->derivative * PIDVal->Kd;
  PIDVal->I = PIDVal->integral * PIDVal->Ki;

  // PID
  PIDVal->output = PIDVal->P + PIDVal->I + PIDVal->D;

  // Calculo de velocidade do motor
  carVal->speed.rightActual = carVal->speed.rightBase - PIDVal->output;
  carVal->speed.leftActual = carVal->speed.leftBase + PIDVal->output;
}

////////////////END FUNÇÕES////////////////

////////////////START TASKS////////////////

void vTaskMotors(void *pvParameters)
{

  ESP32MotorControl motdrv;
  motdrv.setSTBY(DRIVER_STBY);

  motdrv.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1,
                      DRIVER_BIN2, DRIVER_PWMB);

  vTaskSuspend(xTaskMotors);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    MotorControlFunc(&motdrv, ((valuesCar *)pvParameters));
    //vTaskDelay(1);
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskSensors(void *pvParameters)
{
  // Instancia dos sensores de linha
  QTRSensors array;
  QTRSensors s_laterais;

  // Instancia dos encoders
  ESP32Encoder enc_dir;
  ESP32Encoder enc_esq;

  // Anexa os IOs a instancia do contador dos encoders
  enc_dir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);
  enc_esq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);

  // Inicializa o array conectado ao ADC MCP3008
  array.setTypeMCP3008();
  array.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8,
                      GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_22, 1350000,
                      VSPI_HOST);

  // Inicializa os sensores laterais conectados no ADC do ESP32
  s_laterais.setTypeAnalogESP();
  s_laterais.setSensorPins((const adc1_channel_t[]){SL1, SL3}, 2);

  gpio_set_level(GPIO_NUM_2, 1);

  calibSensor(&array);

  for (uint8_t i = 2; i < 12; i++)
  {
    gpio_set_level(GPIO_NUM_2, i % 2);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  calibSensor(&s_laterais);

  gpio_set_level(GPIO_NUM_2, 0);

  vTaskResume(xTaskMotors);
  vTaskResume(xTaskPID);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    GetData(&array, &s_laterais, &enc_dir, &enc_esq, ((valuesCar *)pvParameters));
    processSLat(((valuesCar *)pvParameters));
    //vTaskDelay(1);
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskPID(void *pvParameters)
{
  // PID configs

  valuesPID PIDVal;
  PIDVal.input = &(((valuesCar *)pvParameters)->sArray.line);

  vTaskSuspend(xTaskPID);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    PIDFollow(&PIDVal, ((valuesCar *)pvParameters));
    //vTaskDelay(1);
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskFTP(void *pvParameters)
{
  if (SPIFFS.begin(true))
  {
    ESP_LOGD("SPIFFS", "SPIFFS Iniciada!");
    ftpSrv.begin("Braia", "W2knaft@123"); //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)
  }

  for (;;)
  {
    ftpSrv.handleFTP();
    vTaskDelay(1);
  }
}

////////////////END TASKS////////////////

void app_main(void)
{
  initArduino();
  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);

  ESP_LOGD("WiFi", "Conectando na rede");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
    ESP_LOGD("WiFi", ".");
  ESP_LOGD("WiFi", "Conetado!, IP: %s", WiFi.localIP().toString().c_str());

  WiFi.setSleep(false);

  // Instancia o carro
  valuesCar braiaVal;

  // Alocando espaço de memória para salvar os valores dos canais do array e dos sensores laterais
  braiaVal.sArray.channel = (uint16_t *)heap_caps_calloc(8, sizeof(uint16_t), MALLOC_CAP_8BIT);
  braiaVal.sLat.channel = (uint16_t *)heap_caps_calloc(2, sizeof(uint16_t), MALLOC_CAP_8BIT);

  esp_log_level_set("ESP32MotorControl", ESP_LOG_ERROR);

  xTaskCreate(vTaskSensors, "TaskSensors", 10000, &braiaVal, 10, &xTaskSensors);
  xTaskCreate(vTaskPID, "TaskPID", 10000, &braiaVal, 9, &xTaskPID);
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, &braiaVal, 8, &xTaskMotors);
  xTaskCreate(vTaskFTP, "TaskFTP", 10000, &braiaVal, 4, &xTaskFTP);

  for (;;)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP_LOGD("Braia", "Linha: %.lf\tEncs: %d\tSpeedL: %.2lf\tSpeedR: %.2lf", braiaVal.sArray.line, braiaVal.motEncs.media, braiaVal.speed.leftActual, braiaVal.speed.rightActual);
  }
}