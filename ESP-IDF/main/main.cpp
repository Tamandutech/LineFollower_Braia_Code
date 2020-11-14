#include "includes.hpp"
#include "dataStructs.hpp"

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

// Listas para envio de dados ESP-NOW
list<valuesSamples> valuesToESPNOW;
list<paramsSamples> paramsToESPNOW;

// Objetos de recebimento de dados ESP-NOW
valuesSamples valueFromESPNOW;
paramsSamples paramFromESPNOW;

TaskHandle_t xTaskMotors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskFTP;
TaskHandle_t xTaskESPNOW;
TaskHandle_t xTaskVerifyState;

SemaphoreHandle_t xSemaphoreCarValues = NULL;
SemaphoreHandle_t xSemaphoreCarParams = NULL;
SemaphoreHandle_t xSemaphoreCarValuesToESPNOW = NULL;
SemaphoreHandle_t xSemaphoreCarParamsToESPNOW = NULL;

dataCar braia;

////////////////START FUNÇÕES////////////////
void takeSample(list<valuesSamples> *list, valuesCar *carVal)
{
  if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)10) == pdTRUE && xSemaphoreTake(xSemaphoreCarValuesToESPNOW, (TickType_t)10) == pdTRUE)
  {
    if (list->size() < 100)
      list->emplace_back(valuesSamples(*carVal, esp_log_timestamp()));

    xSemaphoreGive(xSemaphoreCarValues);
    xSemaphoreGive(xSemaphoreCarValuesToESPNOW);
  }
}

void takeSample(list<paramsSamples> *list, paramsCar *carParam)
{
  if (xSemaphoreTake(xSemaphoreCarParams, (TickType_t)10) == pdTRUE && xSemaphoreTake(xSemaphoreCarParamsToESPNOW, (TickType_t)10) == pdTRUE)
  {
    if (list->size() < 100)
      list->emplace_back(paramsSamples(*carParam, esp_log_timestamp()));

    xSemaphoreGive(xSemaphoreCarParams);
    xSemaphoreGive(xSemaphoreCarParamsToESPNOW);
  }
}

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

void calibAllSensors(QTRSensors *array, QTRSensors *s_laterais, paramsCar *carParam)
{
  gpio_set_level(GPIO_NUM_2, 1);

  calibSensor(array);

  if (xSemaphoreTake(xSemaphoreCarParams, (TickType_t)10) == pdTRUE)
  {
    for (uint8_t i = 0; i < array->getSensorCount(); i++)
    {
      carParam->sArray.maxChannel[i] = array->calibrationOn.maximum[i];
      carParam->sArray.minChannel[i] = array->calibrationOn.minimum[i];
    }

    xSemaphoreGive(xSemaphoreCarParams);
  }

  for (uint8_t i = 2; i < 12; i++)
  {
    gpio_set_level(GPIO_NUM_2, i % 2);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  calibSensor(s_laterais);

  if (xSemaphoreTake(xSemaphoreCarParams, (TickType_t)10) == pdTRUE)
  {
    for (uint8_t i = 0; i < s_laterais->getSensorCount(); i++)
    {
      carParam->sLat.maxChannel[i] = s_laterais->calibrationOn.maximum[i];
      carParam->sLat.minChannel[i] = s_laterais->calibrationOn.minimum[i];
    }
    gpio_set_level(GPIO_NUM_2, 0);

    carParam->validParams = (carParam->validParams | SLAT_VALID | SARRAY_VALID);

    xSemaphoreGive(xSemaphoreCarParams);
  }

  takeSample(&paramsToESPNOW, carParam);
}

void getSensors(QTRSensors *array, QTRSensors *s_laterais, ESP32Encoder *enc_dir, ESP32Encoder *enc_esq, valuesCar *carVal)
{
  if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)10) == pdTRUE)
  {
    // Update erro do array
    carVal->sArray.line = array->readLineWhite(carVal->sArray.channel);

    xSemaphoreGive(xSemaphoreCarValues);
  }

  if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)10) == pdTRUE)
  {
    // Update dos sensores laterais
    s_laterais->readCalibrated(carVal->sLat.channel);
    carVal->sLat.line = carVal->sLat.channel[0] - carVal->sLat.channel[1];

    xSemaphoreGive(xSemaphoreCarValues);
  }

  if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)10) == pdTRUE)
  {
    // Update dos encoders dos motores
    carVal->motEncs.encDir = enc_dir->getCount() * -1;
    carVal->motEncs.encEsq = enc_esq->getCount();

    xSemaphoreGive(xSemaphoreCarValues);
  }
}

void MotorControlFunc(ESP32MotorControl *MotorControl, valuesCar *carVal, paramsCar *carParam)
{
  if (carVal->state != CAR_STOPPED)
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
  if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)10) == pdTRUE)
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

    xSemaphoreGive(xSemaphoreCarValues);
  }
}

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

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  /* Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail"); */
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  if (len == sizeof(enum_espNOW))
  {
    enum_espNOW cmdESPNOW;
    memcpy(&cmdESPNOW, incomingData, sizeof(cmdESPNOW));

    ESP_LOGD("ESP-NOW", "Bytes cmdESPNOW: %d", len);

    switch (cmdESPNOW)
    {
    case REQUEST_CARVALUES:
      ESP_LOGD("ESP-NOW", "Valores solicitados pela central, adicionando na fila");
      takeSample(&valuesToESPNOW, &braia.values);
      ESP_LOGD("ESP-NOW", "Adicionado com sucesso");

      break;

    case REQUEST_CARPARAM:
      ESP_LOGD("ESP-NOW", "Parâmetros solicitados pela central, adicionando na fila");
      takeSample(&paramsToESPNOW, &braia.params);
      ESP_LOGD("ESP-NOW", "Adicionado com sucesso");

      break;

    default:
      break;
    }
  }

  else if (len == sizeof(paramFromESPNOW))
  {
    ESP_LOGD("ESP-NOW", "Sample de parâmetros recebido, bytes: %d", len);

    ESP_LOGD("ESP-NOW", "Tentando implementar sample");
    if (xSemaphoreTake(xSemaphoreCarParams, (TickType_t)40) == pdTRUE)
    {
      memcpy(&braia.params, incomingData, sizeof(braia.params));
      ESP_LOGD("ESP-NOW", "Sample implementado com sucesso!");

      string maxsmins = "";
      for (uint8_t i = 0; i < 8; i++)
        maxsmins += to_string(braia.params.sArray.minChannel[i]) + ' ';

      maxsmins += '\n';

      for (uint8_t i = 0; i < 8; i++)
        maxsmins += to_string(braia.params.sArray.maxChannel[i]) + ' ';

      ESP_LOGD("QTRSensors", "\n%s", maxsmins.c_str());

      maxsmins = "";
      for (uint8_t i = 0; i < 2; i++)
        maxsmins += to_string(braia.params.sLat.minChannel[i]) + ' ';

      maxsmins += '\n';

      for (uint8_t i = 0; i < 2; i++)
        maxsmins += to_string(braia.params.sLat.maxChannel[i]) + ' ';

      ESP_LOGD("PARAM-QTRSENSORS", "\n%s", maxsmins.c_str());

      ESP_LOGD("PARAM-SPEED", "Speed -> Max: %d | Min: %d | Base: %d", braia.params.speed.atual->max, braia.params.speed.atual->min, braia.params.speed.atual->base);

      xSemaphoreGive(xSemaphoreCarParams);
    }
  }

  else if (len == sizeof(valueFromESPNOW))
  {
    ESP_LOGD("ESP-NOW", "Sample de valores recebido, bytes: %d", len);

    ESP_LOGD("ESP-NOW", "Tentando salvar sample na lista");
    if (xSemaphoreTake(xSemaphoreCarValues, (TickType_t)40) == pdTRUE)
    {
      memcpy(&braia.values, incomingData, sizeof(braia.values));

      ESP_LOGD("ESP-NOW", "Sample salvo com sucesso!");

      xSemaphoreGive(xSemaphoreCarValues);
    }
  }
}

////////////////END FUNÇÕES////////////////

////////////////START TASKS////////////////

void vTaskVerifyState(void *pvParameters)
{
  valuesCar *carVal = &((dataCar *)pvParameters)->values;
  paramsCar *carParam = &((dataCar *)pvParameters)->params;

  if (carVal->latMarks.rightPassed < TRACK_RIGHT_MARKS /* || carVal.leftMarksPassed < TRACK_LEFT_MARKS */)
    if (carVal->latMarks.leftPassed % 2 != 0)
    {
      carVal->state = CAR_IN_CURVE;
      carParam->PID.atual = &carParam->PID.reta;
      carParam->speed.atual = &carParam->speed.reta;
    }
    else
    {
      carVal->state = CAR_IN_LINE;
      carParam->PID.atual = &carParam->PID.curva;
      carParam->speed.atual = &carParam->speed.curva;
    }
  else
    carVal->state = CAR_STOPPED;
}

void vTaskMotors(void *pvParameters)
{

  ESP32MotorControl motdrv;
  motdrv.setSTBY(DRIVER_STBY);

  motdrv.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1,
                      DRIVER_BIN2, DRIVER_PWMB);

  ESP_LOGD("vTaskMotors", "Pausando...");
  vTaskSuspend(xTaskMotors);
  ESP_LOGD("vTaskMotors", "Retomada!");

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
  paramsCar *carParam = &((dataCar *)pvParameters)->params;

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

  // Pausa task para aguardar resposta do servidor
  ESP_LOGD("vTaskSensors", "Pausando...");
  vTaskSuspend(xTaskSensors);
  ESP_LOGD("vTaskSensors", "Retomada!");

  // Verifica se central mandou parametros
  if ((carParam->validParams & (SARRAY_VALID | SLAT_VALID)) != 0)
  {
    // Caso a central mande os parametros, pegue a ultima calibracao
    ESP_LOGD("vTaskSensors", "Definindo parâmetros recebidos");
    array.setCalibrationOn(carParam->sArray.maxChannel, carParam->sArray.minChannel);
    s_laterais.setCalibrationOn(carParam->sLat.maxChannel, carParam->sLat.minChannel);
  }
  else
  {
    // Caso a central nao mande os parametros, calibre
    ESP_LOGD("vTaskSensors", "Sem parâmetros de calibração, calibrando...");
    calibAllSensors(&array, &s_laterais, carParam);
  }

  // Inicia loop das outras tasks
  vTaskResume(xTaskMotors);
  vTaskResume(xTaskPID);
  vTaskResume(xTaskCarStatus);

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

  ESP_LOGD("vTaskPID", "Pausando...");
  vTaskSuspend(xTaskPID);
  ESP_LOGD("vTaskPID", "Retomada!");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    // Calculo de valor lido
    proportional = static_cast<float>(*carVal->PID.input) - 3500;
    derivative = proportional - last_proportional;
    integral = integral + proportional;
    last_proportional = proportional;

    P = proportional * carParam->PID.atual->Kp;
    D = derivative * carParam->PID.atual->Kd;
    I = integral * carParam->PID.atual->Ki;

    // PID
    carVal->PID.output = P + I + D;

    // Calculo de velocidade do motor

    carVal->speed.right = constrain(carParam->speed.atual->base - static_cast<int8_t>(carVal->PID.output), carParam->speed.atual->min, carParam->speed.atual->max);
    carVal->speed.left = constrain(carParam->speed.atual->base + static_cast<int8_t>(carVal->PID.output), carParam->speed.atual->min, carParam->speed.atual->max);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS);
  }
}

void vTaskCarStatus(void *pvParameters)
{
  valuesCar *carVal = &((dataCar *)pvParameters)->values;
  //paramsCar *carParam = &((dataCar *)pvParameters)->params;

  ESP_LOGD("vTaskCarStatus", "Pausando...");
  vTaskSuspend(xTaskCarStatus);
  ESP_LOGD("vTaskCarStatus", "Retomada!");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    takeSample(&valuesToESPNOW, carVal);

    vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
  }
}

void vTaskESPNOW(void *pvParameters)
{
  //valuesCar *carVal = &((dataCar *)pvParameters)->values;

  enum_espNOW cmdESPNOW;

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

  esp_now_register_recv_cb(OnDataRecv);

  // Requisita os parametros para a central
  ESP_LOGD("ESP-NOW", "Solicitando parâmetros para Central");
  cmdESPNOW = REQUEST_CARPARAM;
  esp_now_send(broadcastAddress, (uint8_t *)&cmdESPNOW, sizeof(cmdESPNOW));

  ESP_LOGD("ESP-NOW", "Aguardando 3s");
  vTaskDelay(3000 / portTICK_PERIOD_MS);

  vTaskResume(xTaskSensors);

  for (;;)
  {
    // Envio de valores
    if (xSemaphoreTake(xSemaphoreCarValuesToESPNOW, (TickType_t)20) == pdTRUE)
    {
      if (valuesToESPNOW.size() > 0)
      {
        if (esp_now_send(broadcastAddress, (uint8_t *)&valuesToESPNOW.front(), sizeof(valuesToESPNOW.front())) == ESP_OK)
        {
          ESP_LOGD("ESP-NOW", "Sample de valores enviado para central, bytes: %d", sizeof(valuesToESPNOW.front()));
          valuesToESPNOW.pop_front();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      xSemaphoreGive(xSemaphoreCarValuesToESPNOW);
    }

    // Envio de parâmetros
    if (xSemaphoreTake(xSemaphoreCarParamsToESPNOW, (TickType_t)20) == pdTRUE)
    {
      if (paramsToESPNOW.size() > 0)
      {
        if (esp_now_send(broadcastAddress, (uint8_t *)&paramsToESPNOW.front(), sizeof(paramsToESPNOW.front())) == ESP_OK)
        {
          ESP_LOGD("ESP-NOW", "Sample de parâmetros enviado para central, bytes: %d", sizeof(paramsToESPNOW.front()));
          paramsToESPNOW.pop_front();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
      xSemaphoreGive(xSemaphoreCarParamsToESPNOW);
    }
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

  ESP_LOGD("Memoria", "Carro: %d\tParametros: %d\tValores: %d", sizeof(braia), sizeof(braia.params), sizeof(braia.values));

  esp_log_level_set("ESP32MotorControl", ESP_LOG_ERROR);

  vSemaphoreCreateBinary(xSemaphoreCarValues);
  vSemaphoreCreateBinary(xSemaphoreCarParams);
  vSemaphoreCreateBinary(xSemaphoreCarValuesToESPNOW);
  vSemaphoreCreateBinary(xSemaphoreCarParamsToESPNOW);

  xTaskCreate(vTaskVerifyState, "TaskVerifyState", 10000, &braia, 9, &xTaskVerifyState);
  xTaskCreate(vTaskESPNOW, "TaskESPNOW", 10000, &braia, 6, &xTaskESPNOW);
  xTaskCreate(vTaskSensors, "TaskSensors", 10000, &braia, 10, &xTaskSensors);
  xTaskCreate(vTaskPID, "TaskPID", 10000, &braia, 9, &xTaskPID);
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, &braia, 8, &xTaskMotors);
  xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, &braia, 7, &xTaskCarStatus);

  for (;;)
  {
    /* ESP_LOGD("WaterMark", "\n-ESPNOW: %d\n-Sensors: %d\n-PID: %d\n-Motors: %d\n-CarStatus: %d",
             uxTaskGetStackHighWaterMark(xTaskESPNOW), uxTaskGetStackHighWaterMark(xTaskSensors), uxTaskGetStackHighWaterMark(xTaskPID),
             uxTaskGetStackHighWaterMark(xTaskMotors), uxTaskGetStackHighWaterMark(xTaskCarStatus));
 */
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}