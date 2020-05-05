#include <stdbool.h>
#include <string>
#include <list>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"

#include "ESP32Encoder.h"
#include "ESP32MotorControl.h"
#include "QTRSensors.h"
#include "esp_log.h"
#include "io.h"

#define LINE LINE_BLACK
#define TRACK_LEFT_MARKS 57
#define TRACK_RIGHT_MARKS 12

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

using namespace std;

// MACs ESPs:

// Braia: 24:6F:28:B2:23:D0
// CCenter: 24:6F:28:9D:7C:44

uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x9D, 0x7C, 0x44};
esp_now_peer_info_t peerInfo;

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
  int16_t line;
};

struct valuesEnc
{
  int32_t encDir = 0;
  int32_t encEsq = 0;
};

struct valuesMarks
{
  int16_t leftPassed = 0;
  int16_t rightPassed = 0;
  bool sLatEsq;
  bool sLatDir;
};

struct valuesPID
{
  // Parâmetros
  int16_t *input = NULL;
  float output = 0;
};

struct valuesSpeed
{
  int8_t right = 0;
  int8_t left = 0;
};

struct paramSpeed
{
  int8_t max = 80;
  int8_t min = 5;
  int8_t base = 40;
};

struct paramPID
{
  int16_t setpoint = 3500;
  float Kp = 0.01;
  float Ki = 0.00;
  float Kd = 0.10;
};

struct valuesCar
{
  int8_t state = 1; // 0: parado, 1: linha, 2: curva
  valuesSpeed speed;
  valuesPID PID;
  valuesMarks latMarks;
  valuesEnc motEncs;
  valuesS sLat;
  valuesS sArray;
};

struct paramsCar
{
  paramSpeed speed;
  paramPID PID;
};

struct dataCar
{
  valuesCar values;
  paramsCar params;
};

struct valuesSamples
{
  valuesCar carVal;
  uint32_t time;

  valuesSamples(valuesCar carVal) : carVal(carVal)
  {
    time = esp_log_timestamp();
  };

  valuesSamples(){};
};

list<valuesSamples> valSamples;

TaskHandle_t xTaskMotors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskFTP;
TaskHandle_t xTaskESPNOW;

////////////////START FUNÇÕES////////////////

void calibSensor(QTRSensors *sensor)
{
  std::string maxsmins = "";

  for (uint16_t i = 0; i < 200; i++)
  {
    sensor->calibrate();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  for (uint8_t i = 0; i < sensor->getSensorCount(); i++)
    maxsmins += to_string(sensor->calibrationOn.minimum[i]) + ' ';

  maxsmins += '\n';

  for (uint8_t i = 0; i < sensor->getSensorCount(); i++)
    maxsmins += to_string(sensor->calibrationOn.maximum[i]) + ' ';

  ESP_LOGD("QTRSensors", "\n%s", maxsmins.c_str());
}

void getSensors(QTRSensors *array, QTRSensors *s_laterais, ESP32Encoder *enc_dir, ESP32Encoder *enc_esq, valuesCar *carVal)
{
  // Update erro do array
  carVal->sArray.line = array->readLineWhite(carVal->sArray.channel);

  // Update dos sensores laterais
  s_laterais->readCalibrated(carVal->sLat.channel);
  carVal->sLat.line = carVal->sLat.channel[0] - carVal->sLat.channel[1];

  // Update dos encoders dos motores
  carVal->motEncs.encDir = enc_dir->getCount() * -1;
  carVal->motEncs.encEsq = enc_esq->getCount();
}

void MotorControlFunc(ESP32MotorControl *MotorControl, valuesCar *carVal, paramsCar *carParam)
{
  if (carVal->state != 0)
  {
    MotorControl->motorForward(0);
    MotorControl->motorForward(1);
    MotorControl->motorSpeed(1, carVal->speed.left);
    MotorControl->motorSpeed(0, carVal->speed.right);
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
        carVal->latMarks.leftPassed++;

      carVal->latMarks.sLatEsq = true;
      carVal->latMarks.sLatDir = false;
    }
    else
    {
      // Debounce
      if (!(carVal->latMarks.sLatDir))
        carVal->latMarks.rightPassed++;

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

/* WiFi should start before using ESPNOW */
static void wifiInit(void)
{
  tcpip_adapter_init();
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  esp_wifi_set_ps(WIFI_PS_NONE);
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
}

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  /* Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"); */
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  /*  memcpy(&incomingSample, incomingData, sizeof(incomingSample));
  Serial.print("Bytes received: ");
  Serial.println(len);

  samplesList.push_back(incomingSample); */
}

void takeSample(list<valuesSamples> *list, valuesCar *carVal)
{
  if (list->size() < 100)
    list->emplace_back(valuesSamples(*carVal));
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
  valuesCar *carVal = &((dataCar *)pvParameters)->values;

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
    getSensors(&array, &s_laterais, &enc_dir, &enc_esq, carVal);
    processSLat(carVal);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskPID(void *pvParameters)
{
  valuesCar *carVal = &((dataCar *)pvParameters)->values;
  paramsCar *carParam = &((dataCar *)pvParameters)->params;

  // Variaveis de calculo
  float P = 0, I = 0, D = 0;
  float last_proportional = 0;
  float proportional = 0;
  float derivative = 0;
  float integral = 0;

  // PID configs
  carVal->PID.input = &carVal->sArray.line;

  vTaskSuspend(xTaskPID);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    // Calculo de valor lido
    proportional = static_cast<float>(*carVal->PID.input) - 3500;
    derivative = proportional - last_proportional;
    integral = integral + proportional;
    last_proportional = proportional;

    P = proportional * carParam->PID.Kp;
    D = derivative * carParam->PID.Kd;
    I = integral * carParam->PID.Ki;

    // PID
    carVal->PID.output = P + I + D;

    // Calculo de velocidade do motor

    carVal->speed.right = constrain(carParam->speed.base - static_cast<int8_t>(carVal->PID.output), carParam->speed.min, carParam->speed.max);
    carVal->speed.left = constrain(carParam->speed.base + static_cast<int8_t>(carVal->PID.output), carParam->speed.min, carParam->speed.max);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskCarStatus(void *pvParameters)
{
  valuesCar *carVal = &((dataCar *)pvParameters)->values;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    takeSample(&valSamples, carVal);

    if (carVal->latMarks.sLatDir || carVal->latMarks.sLatEsq)
      vTaskDelayUntil(&xLastWakeTime, 20 / portTICK_PERIOD_MS);
    else
      vTaskDelayUntil(&xLastWakeTime, 200 / portTICK_PERIOD_MS);
  }
}

void vTaskFTP(void *pvParameters)
{
  for (;;)
  {
    vTaskDelay(1);
  }
}

void vTaskESPNOW(void *pvParameters)
{
  valuesCar *carVal = &((dataCar *)pvParameters)->values;

  if (esp_now_init() != 0)
    ESP_LOGD("ESP-NOW", "Falha ao iniciar");

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 11;
  peerInfo.encrypt = false;
  peerInfo.ifidx = ESP_IF_WIFI_STA;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    ESP_LOGD("ESP-NOW", "Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    if (valSamples.size() > 0)
      if (esp_now_send(broadcastAddress, (uint8_t *)&valSamples.front(), sizeof(valSamples.front())) == ESP_OK)
        valSamples.pop_front();

    if (valSamples.size() > 10)
      vTaskDelayUntil(&xLastWakeTime, 50 / portTICK_PERIOD_MS);
    else
      vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
  }
}

////////////////END TASKS////////////////

void app_main(void)
{

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifiInit();

  gpio_set_direction(GPIO_NUM_2, GPIO_MODE_INPUT_OUTPUT);

  // Instancia o carro
  dataCar braia;

  ESP_LOGD("Memoria", "Carro: %d\tParametros: %d\tValores: %d", sizeof(braia), sizeof(braia.params), sizeof(braia.values));

  // Alocando espaço de memória para salvar os valores dos canais do array e dos sensores laterais
  braia.values.sArray.channel = (uint16_t *)heap_caps_calloc(8, sizeof(uint16_t), MALLOC_CAP_8BIT);
  braia.values.sLat.channel = (uint16_t *)heap_caps_calloc(2, sizeof(uint16_t), MALLOC_CAP_8BIT);

  esp_log_level_set("ESP32MotorControl", ESP_LOG_ERROR);

  xTaskCreate(vTaskESPNOW, "TaskESPNOW", 10000, &braia, 6, &xTaskMotors);
  xTaskCreate(vTaskSensors, "TaskSensors", 10000, &braia, 10, &xTaskSensors);
  xTaskCreate(vTaskPID, "TaskPID", 10000, &braia, 9, &xTaskPID);
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, &braia, 8, &xTaskMotors);
  xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, &braia, 7, &xTaskCarStatus);
  //xTaskCreate(vTaskFTP, "TaskFTP", 10000, &braia, 4, &xTaskFTP);

  for (;;)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP_LOGD("Braia", "Linha: %d\tEncs: %d\tSpeedL: %d\tSpeedR: %d", braia.values.sArray.line, (braia.values.motEncs.encDir + braia.values.motEncs.encEsq) / 2, braia.values.speed.left, braia.values.speed.right);
  }
}