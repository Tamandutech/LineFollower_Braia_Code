#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"

extern "C"
{
  void app_main(void);
}

QTRSensors array;
QTRSensors s_laterais;

// Instancia dos encodes
ESP32Encoder enc_esq;
ESP32Encoder enc_dir;

const uint8_t SensorCount = 8;

uint16_t sensorValues[SensorCount];

ESP32MotorControl motdrv;

TaskHandle_t xTaskMotors;
TaskHandle_t xTaskEncoders;
TaskHandle_t xTaskSensors;

void vTaskMotors(void *pvParameters)
{
  motdrv.setSTBY(DRIVER_STBY);

  motdrv.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1,
                      DRIVER_BIN2, DRIVER_PWMB);

  for (;;)
  {
    motdrv.motorForward(0);
    motdrv.motorForward(1);

    for (float i = 0; i < 101; i += 0.5)
    {
      motdrv.motorSpeed(0, i);
      motdrv.motorSpeed(1, i);
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    motdrv.motorsStop();

    vTaskDelay(500 / portTICK_PERIOD_MS);

    motdrv.motorReverse(0);
    motdrv.motorReverse(1);

    for (float i = 0; i < 101; i += 0.5)
    {
      motdrv.motorSpeed(0, i);
      motdrv.motorSpeed(1, i);
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    motdrv.motorsStop();

    vTaskSuspend(xTaskMotors);
  }
}

void vTaskEncoders(void *pvParameters)
{
  // Anexa os IOs a instancia do contador dos encoders
  enc_dir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);
  enc_esq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    ESP_LOGD("Encoders", "esq: %d\tdir: %d", enc_esq.getCount(),
             enc_dir.getCount());
    vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
  }
}

void vTaskSensors(void *pvParameters)
{
  // Inicializa o Driver do MCP3008
  array.setTypeMCP3008();
  array.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, SensorCount,
                      GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_22, 1350000,
                      VSPI_HOST);

  s_laterais.setTypeAnalogESP();
  s_laterais.setSensorPins((const adc1_channel_t[]){SL1, SL3}, 2);

  uint16_t sLatValues[4];

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    s_laterais.read(sLatValues);
    ESP_LOGD("SenLaterias", "SLE: %d\tSLD: %d", sLatValues[0],
             sLatValues[1]);
    vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);
  gpio_set_level(GPIO_NUM_2, 1);

  //xTaskCreate(vTaskEncoders, "TaskEncoders", 10000, NULL, 2, &xTaskEncoders);
  //xTaskCreate(vTaskMotors, "TaskMotors", 10000, NULL, 2, &xTaskMotors);
  xTaskCreate(vTaskSensors, "TaskSensors", 10000, NULL, 2, &xTaskSensors);

  /* gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);
  gpio_set_level(GPIO_NUM_2, 1);

  for (uint16_t i = 0; i < 400; i++) {
    qtr.calibrate();
  }

  gpio_set_level(GPIO_NUM_2, 0);

  for (uint8_t i = 0; i < SensorCount; i++) {
    printf("%d", qtr.calibrationOn.minimum[i]);
    printf(" ");
  }
  printf("\n");

  // print the calibration maximum values measured when emitters were on
  for (uint8_t i = 0; i < SensorCount; i++) {
    printf("%d", qtr.calibrationOn.maximum[i]);
    printf(" ");
  }
  printf("\n");
  printf("\n");

  vTaskDelay(1000 / portTICK_PERIOD_MS);
*/
}