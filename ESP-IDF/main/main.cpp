#include "QTRSensors.h"
#include "esp_log.h"

extern "C" {
void app_main(void);
}

QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

void app_main(void) {
  // Inicializa o Driver do MCP3008
  qtr.setTypeMCP3008();
  qtr.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, SensorCount, GPIO_NUM_19,
                    GPIO_NUM_23, GPIO_NUM_18, GPIO_NUM_22, 1350000, VSPI_HOST);

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

  int64_t time = 0;

  while (1) {
    // read calibrated sensor values and obtain a measure of the line position
    // from 0 to 5000 (for a white line, use readLineWhite() instead)
    int16_t position = qtr.readLineWhite(sensorValues);

    // print the sensor values as numbers from 0 to 1000, where 0 means maximum
    // reflectance and 1000 means minimum reflectance, followed by the line
    // position
    time = esp_timer_get_time();

    for (uint8_t i = 0; i < SensorCount; i++) {
      printf("%d", sensorValues[i]);
      printf("\t");
    }
    
    printf("%d", position);
    printf("\n");
    
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}