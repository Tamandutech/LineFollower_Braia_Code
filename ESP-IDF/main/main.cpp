#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"

extern "C" {
void app_main(void);
}

QTRSensors qtr;

// Instancia dos encodes
ESP32Encoder enc_esq;
ESP32Encoder enc_dir;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

void app_main(void) {
  // Inicializa o Driver do MCP3008
  qtr.setTypeMCP3008();
  qtr.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, SensorCount,
                    GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_22, 1350000,
                    VSPI_HOST);

  // Anexa os IOs a instancia do contador dos encoders
  enc_dir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);
  enc_esq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);
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

  while (1) {

    printf("%d", enc_dir.getCount());
    printf("\t");
    printf("%d", enc_esq.getCount());
    printf("\t");
    printf("\n");

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}