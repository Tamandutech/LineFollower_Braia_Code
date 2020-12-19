#include "includes.hpp"
#include "RobotData.h"

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Componentes de encapsulamento das variaveis
Robot *braia;

// TaksHandles para gerenciar execucao das tasks
TaskHandle_t xTaskMotors;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskSpeed;

/////////////// INICIO TASKs DO ROBO ///////////////

void vTaskMotors(void *pvParameters)
{
  static const char *TAG = "vTaskMotors";
  //Robot *braia = (Robot *)pvParameters;
  // Setup
  ESP_LOGD(TAG, "Task criada!");

  // Componente de controle dos motores
  ESP32MotorControl motors;

  // GPIOs dos motores
  motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1, DRIVER_BIN2, DRIVER_PWMB);
  motors.setSTBY(DRIVER_STBY);

  // Pausa da Task
  vTaskSuspend(xTaskMotors);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, quarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Loop
  for (;;)
  {

    vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_PERIOD_MS);
  }
}

void vTaskSensors(void *pvParameters)
{
  static const char *TAG = "vTaskSensors";
  //Robot *braia = (Robot *)pvParameters;

  // Setup
  ESP_LOGD(TAG, "Task criada!");

  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  // Definindo GPIOs e configs para sensor Array
  sArray.setTypeMCP3008();
  sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, ADC_DIN, ADC_DOUT, ADC_CLK, ADC_CS, 1350000, VSPI_HOST);
  sArray.setSamplesPerSensor(5);

  // Definindo GPIOs e configs para sensor Lateral
  sLat.setTypeAnalogESP();
  sLat.setSensorPins((const adc1_channel_t[]){SL1, SL2, SL3, SL4}, 4);
  sLat.setSamplesPerSensor(5);

  vTaskSuspend(xTaskSensors);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, quarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Loop
  for (;;)
  {

    vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_PERIOD_MS);
  }
}

void vTaskPID(void *pvParameters)
{
  auto const TaskDelay = 200;
  auto const BaseDeTempo = (TaskDelay * 1E-3);
  auto const h1 = BaseDeTempo / 2;
  auto const h2 = 1 / BaseDeTempo;
  auto const h2x2 = h2 * 2;

  Robot *braia = (Robot *)pvParameters;

  dataPID *PIDTrans = braia->getPIDVel();
  dataPID *PIDRot = braia->getPIDRot();

  // Variaveis de calculo para os pids da velocidade rotacional e translacional
  float KpVel = PIDTrans->getKp(braia->getStatus()->getState());
  float KiVel = PIDTrans->getKi(braia->getStatus()->getState()) * BaseDeTempo;
  float KdVel = PIDTrans->getKd(braia->getStatus()->getState()) / BaseDeTempo;

  float KpRot = PIDRot->getKp(braia->getStatus()->getState());
  float KiRot = PIDRot->getKi(braia->getStatus()->getState()) * BaseDeTempo;
  float KdRot = PIDRot->getKd(braia->getStatus()->getState()) / BaseDeTempo;

  auto VelEncDir = braia->getSpeed()->getRPMRight_inst();
  auto VelEncEsq = braia->getSpeed()->getRPMLeft_inst();

  //erros anteriores
  float errRot_ant = 0, errRot_ant2 = 0;
  float errTrans_ant = 0, errTrans_ant2 = 0;

  //saidas pid anteriores
  float lastTransPid = 0, lastRotPid = 0;

  // Definindo input da classe PID Rotacional valor de linha do sensor Array
  PIDRot->setInput(braia->getsArray()->getLine());

  ESP_LOGD("vTaskPID", "Pausando...");
  vTaskSuspend(xTaskPID);

  ESP_LOGD("vTaskPID", "Retomada!");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    vTaskSuspend(xTaskCarStatus);

    //Velocidade do carrinho
    float VelRot = VelEncDir - VelEncEsq;   // Rotacional
    float VelTrans = VelEncDir + VelEncEsq; //Translacional

    //Erros atuais
    PIDRot->setSetpoint((braia->getsArray()->getLine() - 3500) / 7); // cálculo do setpoint rotacional
    float erroVelTrans = (float)(PIDTrans->getSetpoint()) - VelTrans;
    float erroVelRot = (float)(PIDRot->getSetpoint()) - VelRot;

    //calculando Pids rotacional e translacional
    float PidTrans = erroVelTrans * (KpVel + KiVel * h1 + KdVel * h2) + errTrans_ant * (-KpVel + KiVel * h1 - KdVel * h2x2) + errTrans_ant2 * (KdVel * h2) + lastTransPid;
    errTrans_ant2 = errTrans_ant;
    errTrans_ant = erroVelTrans;
    lastTransPid = PidTrans;
    float PidRot = erroVelRot * (KpRot + KiRot * h1 + KdRot * h2) + errRot_ant * (-KpVel + KiVel * h1 - KdVel * h2x2) + errRot_ant2 * (KdVel * h2) + lastRotPid;
    errRot_ant2 = errRot_ant;
    errRot_ant = erroVelRot;
    lastRotPid = PidRot;

    auto speedBase = braia->getSpeed()->getSpeedBase(braia->getStatus()->getState());
    auto speedMin = braia->getSpeed()->getSpeedMin(braia->getStatus()->getState());
    auto speedMax = braia->getSpeed()->getSpeedMax(braia->getStatus()->getState());

    // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
    PIDTrans->setOutput(constrain(
        ((PidTrans / 10) + speedBase),
        speedMin,
        speedMax));

    PIDRot->setOutput(PidRot / 10);

    // Calculo de velocidade do motor
    braia->getSpeed()->setSpeedRight(
        constrain((int16_t)(PIDTrans->getOutput()) + (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
        braia->getStatus()->getState());

    braia->getSpeed()->setSpeedRight(
        constrain((int16_t)(PIDTrans->getOutput()) - (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
        braia->getStatus()->getState());

    vTaskResume(xTaskCarStatus);
    vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
  }
}

void vTaskCarStatus(void *pvParameters)
{
  static const char *TAG = "vTaskCarStatus";
  //Robot *braia = (Robot *)pvParameters;

  // Setup
  ESP_LOGD(TAG, "Task criada!");

  vTaskSuspend(xTaskCarStatus);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, quarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Loop
  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_PERIOD_MS);
  }
}

/* 
  * Task destinada a calcular velocidade instantanea e media dos motores
  * atraves dos valores de pulsos dos encoders 
  * */
void vTaskSpeed(void *pvParameters)
{
  // Tempo de delay
  auto const TaskDelay = 10;

  static const char *TAG = "vTaskSpeed";

  Robot *braia = (Robot *)pvParameters;
  dataSpeed *speed = braia->getSpeed();

  // Componente de gerenciamento dos encoders
  ESP32Encoder enc_motEsq;
  ESP32Encoder enc_motDir;
  // GPIOs dos encoders dos encoders dos motores
  enc_motEsq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
  enc_motDir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);

  // Quando for começar a utilizar, necessario limpeza da contagem.
  enc_motEsq.clearCount();
  enc_motDir.clearCount();

  TickType_t lastTicksRevsCalc = 0;
  int32_t lastPulseRight = 0;
  int32_t lastPulseLeft = 0;
  // delta entre ultimo calculo e o atual em millisegundos
  uint16_t deltaTimeMS_inst;

  TickType_t initialTicksCar = 0;
  uint16_t deltaTimeMS_media;

  // Setup
  ESP_LOGD(TAG, "Task criada! TaskDelay: %d", TaskDelay);
  vTaskSuspend(xTaskSpeed);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionaliade do vTaskDelayUtil, quarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Variavel contendo quantidade de pulsos inicial do carro
  initialTicksCar = xTaskGetTickCount();
  // Loop
  for (;;)
  {
    deltaTimeMS_inst = (xTaskGetTickCount() - lastTicksRevsCalc) * portTICK_PERIOD_MS;
    lastTicksRevsCalc = xTaskGetTickCount();

    deltaTimeMS_media = (xTaskGetTickCount() - initialTicksCar) * portTICK_PERIOD_MS;

    // Calculos de velocidade instantanea (RPM)
    speed->setRPMLeft_inst(                         // -> Calculo velocidade instantanea motor esquerdo
        (((enc_motEsq.getCount() - lastPulseLeft)   // Delta de pulsos do encoder esquerdo
          / (float)speed->getMPR_MotEsq())                 // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
         / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
         ));
    lastPulseLeft = enc_motEsq.getCount(); // Salva pulsos do encoder para ser usado no proximo calculo

    speed->setRPMRight_inst(                        // -> Calculo velocidade instantanea motor direito
        (((enc_motDir.getCount() - lastPulseRight)  // Delta de pulsos do encoder esquerdo
          / (float)speed->getMPR_MotDir())                 // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
         / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
         ));
    lastPulseRight = enc_motDir.getCount(); // Salva pulsos do motor para ser usado no proximo calculo

    // Calculo de velocidade media do carro (RPM)
    speed->setRPMCar_media(                                                                        // -> Calculo velocidade media do carro
        (((lastPulseRight / (float)speed->getMPR_MotDir() + lastPulseLeft / (float)speed->getMPR_MotEsq())) / 2) // Revolucoes media desde inicializacao
        / ((float)deltaTimeMS_media / (float)60000)                                                // Divisao do delta tempo em minutos para calculo de RPM
    );

    vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
  }
}

/////////////// FIM TASKs DO ROBO ///////////////

extern "C"
{
  void app_main(void);
}

void app_main(void)
{
  // Inicializacao do componente de encapsulamento de dado, definindo nome do robo
  braia = new Robot("Braia");

  // Criacao das tasks e definindo seus parametros
  //xTaskCreate(FUNCAO, NOME, TAMANHO DA HEAP, ARGUMENTO, PRIORIDADE, TASK HANDLE)
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, &braia, 9, &xTaskMotors);
  xTaskCreate(vTaskSensors, "TaskSensors", 10000, &braia, 9, &xTaskSensors);
  xTaskCreate(vTaskPID, "TaskPID", 10000, &braia, 9, &xTaskPID);
  xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, &braia, 9, &xTaskCarStatus);
}