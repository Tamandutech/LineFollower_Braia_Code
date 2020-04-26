#include "stdbool.h"

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

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

extern "C"
{
  void app_main(void);
}

enum CarStatus
{
  IN_CURVE = 0,
  IN_LINE = 1
};

struct valuesS
{
  uint16_t *channel;
  double line;
};

struct valuesEnc
{
  uint32_t encDir = 0;
  uint32_t encEsq = 0;
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

struct valuesPID
{
  // Parâmetros
  double *input = NULL;
  double output = 0;

  // Variaveis de calculo
  float P = 0, I = 0, D = 0;
  float last_proportional = 0;
  float proportional = 0;
  float derivative = 0;
  float integral = 0;
};

struct valuesSpeed
{
  double right = 0;
  double left = 0;
};

struct paramSpeed
{
  uint8_t max = 80;
  uint8_t min = 5;
  uint8_t base = 40;
};

struct paramPID
{
  double setpoint = 3500;
  double outputMin = -100;
  double outputMax = +100;
  double Kp = 0.01;
  double Ki = 0.00;
  double Kd = 0.10;
};

struct valuesCar
{
  int state = 1; // 0: parado, 1: linha, 2: curva
  valuesSpeed speed;
  valuesPID PID;
  valuesSamples *marksTrack;
  valuesMarks latMarks;
  valuesEnc motEncs;
  valuesS sLat;
  valuesS sArray;
};

struct paramsCar{
  paramSpeed speed;
  paramPID PID;
};

struct dataCar{
  valuesCar values;
  paramsCar params;
};

TaskHandle_t xTaskMotors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskFTP;

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

void MotorControlFunc(ESP32MotorControl *MotorControl, valuesCar *carVal, paramsCar *carParam)
{
  if (carVal->state != 0)
  {
    MotorControl->motorForward(0);
    MotorControl->motorForward(1);
    MotorControl->motorSpeed(1, constrain(carVal->speed.left, carParam->speed.min, carParam->speed.max));
    MotorControl->motorSpeed(0, constrain(carVal->speed.right, carParam->speed.min, carParam->speed.max));
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
      // Debounce
      if (!(carVal->latMarks.sLatEsq))
        carVal->sLat.channel[0]++;

      carVal->latMarks.sLatEsq = true;
      carVal->latMarks.sLatDir = false;
    }
    else
    {
      // Debounce
      if (!(carVal->latMarks.sLatDir))
        carVal->sLat.channel[1]++;

      carVal->latMarks.sLatEsq = false;
      carVal->latMarks.sLatDir = true;
    }
  }
  else
  {
    carVal->latMarks.sLatEsq = false;
    carVal->latMarks.sLatDir = false;
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

void PIDFollow(valuesCar *carVal, paramsCar *carParam)
{
  // Calculo de valor lido
  carVal->PID.proportional = (*carVal->PID.input) - 3500;
  carVal->PID.derivative = carVal->PID.proportional - carVal->PID.last_proportional;
  carVal->PID.integral = carVal->PID.integral + carVal->PID.proportional;
  carVal->PID.last_proportional = carVal->PID.proportional;

  carVal->PID.P = carVal->PID.proportional * carParam->PID.Kp;
  carVal->PID.D = carVal->PID.derivative * carParam->PID.Kd;
  carVal->PID.I = carVal->PID.integral * carParam->PID.Ki;

  // PID
  carVal->PID.output = carVal->PID.P + carVal->PID.I + carVal->PID.D;

  // Calculo de velocidade do motor
  carVal->speed.right = carParam->speed.base - carVal->PID.output;
  carVal->speed.left = carParam->speed.base + carVal->PID.output;
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
    MotorControlFunc(&motdrv, &((dataCar *)pvParameters)->values, &((dataCar *)pvParameters)->params);

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
    GetData(&array, &s_laterais, &enc_dir, &enc_esq, &((dataCar *)pvParameters)->values);
    processSLat(&((dataCar *)pvParameters)->values);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskPID(void *pvParameters)
{
  // PID configs
  ((dataCar *)pvParameters)->values.PID.input = &((dataCar *)pvParameters)->values.sArray.line;

  vTaskSuspend(xTaskPID);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    PIDFollow(&((dataCar *)pvParameters)->values, &((dataCar *)pvParameters)->params);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskCarStatus(void *pvParameters)
{

  for (;;)
  {
  }
}

void vTaskFTP(void *pvParameters)
{
  for (;;)
  {
    vTaskDelay(1);
  }
}

////////////////END TASKS////////////////

void app_main(void)
{
  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);

  // Instancia o carro
  dataCar braia;

  // Alocando espaço de memória para salvar os valores dos canais do array e dos sensores laterais
  braia.values.sArray.channel = (uint16_t *)heap_caps_calloc(8, sizeof(uint16_t), MALLOC_CAP_8BIT);
  braia.values.sLat.channel = (uint16_t *)heap_caps_calloc(2, sizeof(uint16_t), MALLOC_CAP_8BIT);

  esp_log_level_set("ESP32MotorControl", ESP_LOG_ERROR);

  xTaskCreate(vTaskSensors, "TaskSensors", 10000, &braia, 10, &xTaskSensors);
  xTaskCreate(vTaskPID, "TaskPID", 10000, &braia, 9, &xTaskPID);
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, &braia, 8, &xTaskMotors);
  //xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, &braia, 4, &xTaskCarStatus);
  //xTaskCreate(vTaskFTP, "TaskFTP", 10000, &braia, 4, &xTaskFTP);

  for (;;)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP_LOGD("Braia", "Linha: %.lf\tEncs: %d\tSpeedL: %.2lf\tSpeedR: %.2lf", braia.values.sArray.line, braia.values.motEncs.media, braia.values.speed.left, braia.values.speed.right);
  }

}