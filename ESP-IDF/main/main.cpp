
#include "includes.hpp"
#include "RobotData.h"
#include "EspNowHandler.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
//#define LINE_COLOR_BLACK
#define taskStatus true // Variável para habilitar a TaskStatus

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// Componentes de encapsulamento das variaveis
Robot *braia;

// TaksHandles para gerenciar execucao das tasks
TaskHandle_t xTaskMotors;
TaskHandle_t xTaskSensors;
TaskHandle_t xTaskPID;
TaskHandle_t xTaskCarStatus;
TaskHandle_t xTaskSpeed;
TaskHandle_t xTaskMapping;
TaskHandle_t xTaskEspNow;

void calibAllsensors(QTRSensors *sArray, QTRSensors *SLat, Robot *braia)
{
  //Calibração dos dos sensores laterais e array
  for (uint16_t i = 0; i < 200; i++)
  {
    sArray->calibrate();
    SLat->calibrate();
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
  //leitura e armazenamento dos valores máximos e mínimos dos sensores obtidos na calibração
  std::vector<uint16_t> sArrayMaxes(sArray->calibrationOn.maximum, sArray->calibrationOn.maximum + sArray->getSensorCount());
  std::vector<uint16_t> sArrayMins(sArray->calibrationOn.minimum, sArray->calibrationOn.minimum + sArray->getSensorCount());
  std::vector<uint16_t> SLatMaxes(SLat->calibrationOn.maximum, SLat->calibrationOn.maximum + SLat->getSensorCount());
  std::vector<uint16_t> SLatMins(SLat->calibrationOn.minimum, SLat->calibrationOn.minimum + SLat->getSensorCount());

  //armazenamento dos valores máximos e mínimos dos sensores no objeto Braia
  braia->getsArray()->setChannelsMaxes(sArrayMaxes);
  braia->getsArray()->setChannelsMins(sArrayMins);
  braia->getsLat()->setChannelsMaxes(SLatMaxes);
  braia->getsLat()->setChannelsMins(SLatMins);
}

void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *braia) // função leitura dos sensores
{
  //Arrays para armazenar leitura bruta dos sensores array e laterais
  uint16_t sArraychannels[sArray->getSensorCount()];
  uint16_t SLatchannels[SLat->getSensorCount()];

#ifdef LINE_COLOR_BLACK
  braia->getsArray()->setLine(sArray->readLineBlack(sArraychannels));  
#else
  braia->getsArray()->setLine(sArray->readLineWhite(sArraychannels));
#endif
                               // cálculo dos valores do sensor array
  SLat->readCalibrated(SLatchannels);                                                                 //leitura dos sensores laterais
  std::vector<uint16_t> sArraychannelsVec(sArraychannels, sArraychannels + sArray->getSensorCount()); // vector(array) com os valores do sensor array
  std::vector<uint16_t> SLatchannelsVec(SLatchannels, SLatchannels + SLat->getSensorCount());         // vector(array) com os valores dos sensores laterais

  //armazenando da leitura bruta do sensor array e lateral no objeto Braia
  braia->getsArray()->setChannels(sArraychannelsVec);
  braia->getsLat()->setChannels(SLatchannelsVec);

  ESP_LOGD("getSensors", "Array: %d | %d | %d | %d | %d | %d | %d | %d ", sArraychannels[0], sArraychannels[1], sArraychannels[2], sArraychannels[3], sArraychannels[4], sArraychannels[5], sArraychannels[6], sArraychannels[7]);
  ESP_LOGD("getSensors", "Linha: %d", braia->getsArray()->getLine());
  ESP_LOGD("getSensors", "Laterais: %d | %d ", SLatchannels[0], SLatchannels[1]);

  //braia->getsLat()->setLine((SLatchannels[0]+SLatchannels[1])/2-(SLatchannels[2]+SLatchannels[3])/2); // cálculo dos valores dos sensores laterais
}

void processSLat(Robot *braia)
{
  bool sldir1 = gpio_get_level(GPIO_NUM_17);
  bool sldir2 = gpio_get_level(GPIO_NUM_5);

  auto SLat = braia->getsLat();
  auto status = braia->getStatus();
  uint16_t slesq1 = SLat->getChannel(0);
  uint16_t slesq2 = SLat->getChannel(1);
  auto latMarks = braia->getSLatMarks();
  ESP_LOGD("processSLat", "Laterais (Direita): %d | %d", sldir1, sldir2);
  ESP_LOGD("processSLat", "Laterais (esquerda): %d | %d", slesq1, slesq2);
  if (slesq1 < 300 || !sldir2 ) // leitura de faixas brancas sensores laterais
  {
    if ((slesq1 < 300) && (sldir2)) //lendo sLat esq. branco e dir. preto
    {
      if (!(latMarks->getSLatEsq())) latMarks->leftPassedInc();
      latMarks->SetSLatEsq(true);
      latMarks->SetSLatDir(false);
      //ESP_LOGI("processSLat", "Laterais (Direita): %d",latMarks->getSLatDir());
    }
    else if ((!sldir2) && (slesq1 > 600)) // lendo sldir. branco e sLat esq. preto 
    {
      if (!(latMarks->getSLatDir())) latMarks->rightPassedInc();
      latMarks->SetSLatDir(true);
      latMarks->SetSLatEsq(false);
    }
  }
  else
  {
    //ESP_LOGI("processSLat", "Laterais (Direita): %d",latMarks->getSLatDir());
    latMarks->SetSLatDir(false);
    latMarks->SetSLatEsq(false);
  }

  if(slesq1 < 300 && !sldir2 && latMarks->getrightMarks() < 2){//continuar em frente em intersecção de linhas
    //braia->getStatus()->setState(CAR_IN_LINE);
  }

  if(latMarks->getrightMarks() >= 2 && status->getMapping()){ //parar depois da leitura da segunda linha direita
    vTaskDelay(500/portTICK_PERIOD_MS);
    vTaskSuspend(xTaskPID);
    braia->getStatus()->setState(CAR_STOPPED);
  }
}

/////////////// INICIO TASKs DO ROBO ///////////////

void vTaskMotors(void *pvParameters)
{
  static const char *TAG = "vTaskMotors";
  //Robot *braia = (Robot *)pvParameters;
  // Setup
  ESP_LOGD(TAG, "Task criada!");

  Robot *braia = (Robot *)pvParameters;
  dataSpeed *speed = braia->getSpeed();
  RobotStatus *status = braia->getStatus();

  // Componente de controle dos motores
  ESP32MotorControl motors;

  // GPIOs dos motores
  motors.attachMotors(DRIVER_AIN1, DRIVER_AIN2, DRIVER_PWMA, DRIVER_BIN1, DRIVER_BIN2, DRIVER_PWMB);
  motors.setSTBY(DRIVER_STBY);

  // Pausa da Task
  vTaskSuspend(xTaskMotors);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();
  //((xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS) < 18000
  //TickType_t xInicialTicks = xTaskGetTickCount();
  // Loop
  for (;;)
  { 
    if (status->getState() != CAR_STOPPED) // verificar se o carrinho deveria se mover
    {
      motors.motorForward(0);                                         // motor 0 ligado para frente
      motors.motorForward(1);                                         // motor 1 ligado para frente
      motors.motorSpeed(0, speed->getSpeedLeft(status->getState()));  // velocidade do motor 0
      motors.motorSpeed(1, speed->getSpeedRight(status->getState())); // velocidade do motor 1
    }
    else
    {
      motors.motorsStop(); // parar motores
      //ESP_LOGE("EncData: ", "%d", mediaEnc);
      //vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS); //10ms
  }
}

void vTaskSensors(void *pvParameters)
{
  static const char *TAG = "vTaskSensors";
  Robot *braia = (Robot *)pvParameters;

  // Setup
  ESP_LOGD(TAG, "Task criada!");

  // Componente de gerenciamento dos sensores
  QTRSensors sArray;
  QTRSensors sLat;

  // Definindo GPIOs e configs para sensor Array
  sArray.setTypeMCP3008();
  sArray.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, 8, (gpio_num_t)ADC_DOUT, (gpio_num_t)ADC_DIN, (gpio_num_t)ADC_CLK, (gpio_num_t)ADC_CS, 1350000, VSPI_HOST);
  sArray.setSamplesPerSensor(5);

  // Definindo GPIOs e configs para sensor Lateral
  gpio_pad_select_gpio(17);
  gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
  gpio_pad_select_gpio(05);
  gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
  sLat.setTypeAnalogESP();
  sLat.setSensorPins((const adc1_channel_t[]){SL1, SL2}, 2);
  sLat.setSamplesPerSensor(5);

  calibAllsensors(&sArray, &sLat, braia); // calibração dos sensores

  vTaskResume(xTaskMotors);
  vTaskResume(xTaskPID);
  //vTaskResume(xTaskEspNow);
  if(taskStatus) vTaskResume(xTaskCarStatus);
  vTaskResume(xTaskSpeed);
  if(braia->getStatus()->getMapping()) vTaskResume(xTaskMapping);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Loop
  for (;;)
  {
    getSensors(&sArray, &sLat, braia); // leitura dos sensores
    processSLat(braia);

    vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_PERIOD_MS); 
  }
}

void vTaskPID(void *pvParameters)
{
  bool DEBUGON = false;
  auto const TaskDelay = 10; // 10ms
  auto const BaseDeTempo = (TaskDelay * 1E-3);
  //auto const h1 = BaseDeTempo / 2;
  //auto const h2 = 1 / BaseDeTempo;
  //auto const h2x2 = h2 * 2;

  Robot *braia = (Robot *)pvParameters;
  dataSpeed *speed = braia->getSpeed();
  RobotStatus *status = braia->getStatus();
  dataPID *PIDTrans = braia->getPIDVel();
  dataPID *PIDRot = braia->getPIDRot();

  // Variaveis de calculo para os pids da velocidade rotacional e translacional
  float KpVel = 0, KiVel = 0, KdVel = 0; 
  float KpRot = 0, KiRot = 0, KdRot = 0;

  //erros anteriores
  float errRot_ant = 0; //errRot_ant2 = 0;
  float errTrans_ant = 0; //errTrans_ant2 = 0;

  //saidas pid anteriores
  //float lastTransPid = 0, lastRotPid = 0;

  //Variáveis para cálculo do pid rot e trans
  float PidTrans=0;
  float Ptrans=0,Itrans=0,Dtrans=0;
  float PidRot = 0; 
  float Prot=0,Irot=0,Drot=0; 
  int iloop = 0;
  // Definindo input da classe PID Rotacional valor de linha do sensor Array
  PIDRot->setInput(braia->getsArray()->getLine());

  ESP_LOGD("vTaskPID", "Pausando...");
  vTaskSuspend(xTaskPID);

  ESP_LOGD("vTaskPID", "Retomada!");

  TickType_t xLastWakeTime = xTaskGetTickCount();
  for (;;)
  {
    CarState estado = status->getState();
    bool mapState = status->getMapping();
    if(iloop>50 && DEBUGON) {
      ESP_LOGI("vTaskPID", "CarstatusOut: %d | bool : %d", estado, mapState);
      ESP_LOGI("vTaskPID", "SetPointTrans: %d", PIDTrans->getSetpoint());
      iloop=0;
    }
    iloop++;
    // Altera a velocidade linear se o carrinho não estiver mapeando
    if ((estado == CAR_IN_LINE) && !mapState) {
      PIDTrans->setSetpoint(1800);
    } 
    else if((estado == CAR_IN_CURVE) && !mapState) {
      PIDTrans->setSetpoint(200);
    }
    // Variaveis de calculo para os pids da velocidade rotacional e translacional
    KpVel = PIDTrans->getKp(estado);
    KiVel = PIDTrans->getKi(estado) * BaseDeTempo;
    KdVel = PIDTrans->getKd(estado) / BaseDeTempo;

    KpRot = PIDRot->getKp(estado);
    KiRot = PIDRot->getKi(estado) * BaseDeTempo;
    KdRot = PIDRot->getKd(estado) / BaseDeTempo;

    //Velocidade do carrinho
    float VelRot = speed->getRPMRight_inst() - speed->getRPMLeft_inst();   // Rotacional
    float VelTrans = speed->getRPMRight_inst() + speed->getRPMLeft_inst(); //Translacional

    //Erros atuais
    PIDRot->setSetpoint((3500 - braia->getsArray()->getLine()) / 7); // cálculo do setpoint rotacional
    float erroVelTrans = (float)(PIDTrans->getSetpoint()) - VelTrans;
    float erroVelRot = (float)(PIDRot->getSetpoint()) - VelRot;

    //calculando Pids rotacional e translacional
    //float PidTrans = erroVelTrans * (KpVel + KiVel * h1 + KdVel * h2) + errTrans_ant * (-KpVel + KiVel * h1 - KdVel * h2x2) + errTrans_ant2 * (KdVel * h2) + lastTransPid;
    //errTrans_ant2 = errTrans_ant;
    Ptrans = KpVel * erroVelTrans;
    Itrans += KiVel * erroVelTrans;
    Dtrans = KdVel * (erroVelTrans - errTrans_ant);
    PidTrans = Ptrans + Itrans + Dtrans;
    errTrans_ant = erroVelTrans;
    //lastTransPid = PidTrans;

    //float PidRot = erroVelRot * (KpRot + KiRot * h1 + KdRot * h2) + errRot_ant * (-KpRot + KiRot * h1 - KdRot * h2x2) + errRot_ant2 * (KdRot * h2) + lastRotPid;
    //errRot_ant2 = errRot_ant;
    Prot = KpRot * erroVelRot;
    Irot += KiRot * erroVelRot;
    Drot = KdRot * (erroVelRot - errRot_ant);
    PidRot = Prot + Irot + Drot;
    errRot_ant = erroVelRot;
    //lastRotPid = PidRot;

    auto speedBase = speed->getSpeedBase(estado);
    auto speedMin = speed->getSpeedMin(estado);
    auto speedMax = speed->getSpeedMax(estado);

    // PID output, resta adequar o valor do Pid para ficar dentro do limite do pwm
    PIDTrans->setOutput(constrain(
        ((PidTrans) + speedBase),
        speedMin,
        speedMax));

    PIDRot->setOutput(PidRot);

    // Calculo de velocidade do motor
    speed->setSpeedRight(
        constrain((int16_t)(PIDTrans->getOutput()) + (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
        estado);

    speed->setSpeedLeft(
        constrain((int16_t)(PIDTrans->getOutput()) - (int16_t)(PIDRot->getOutput()), speedMin, speedMax),
        estado);

    ESP_LOGD("vTaskPID", "speedMin: %d | speedMax: %d", speedMin, speedMax);
    ESP_LOGD("vTaskPID", "PIDRot: %.2f | PIDTrans: %.2f", PIDRot->getOutput(), PIDTrans->getOutput());

    vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
  }
}

void vTaskCarStatus(void *pvParameters)
{
  static const char *TAG = "vTaskCarStatus";
  bool DEBUGON = false;

  Robot *braia = (Robot *)pvParameters;
  RobotStatus *status = braia->getStatus();
  dataSpeed *speed = braia->getSpeed();
  auto latMarks = braia->getSLatMarks();
  auto PidTrans = braia->getPIDVel();

  latMarks->SetTotalLeftMarks(3);
  latMarks->SetFinalMark(5472);
  int Marks = latMarks->getTotalLeftMarks() + 1;  // marcas laterais esquerda na pista 

  struct MapData marktest;
  marktest.MapEncMedia = 0;
  marktest.MapTime = 0;
  marktest.MapStatus = CAR_IN_LINE;
  latMarks->SetMarkDataReg(marktest,0);
  marktest.MapEncMedia = 1100;
  marktest.MapTime = 990;
  marktest.MapStatus = CAR_IN_CURVE;
  latMarks->SetMarkDataReg(marktest,1);
  marktest.MapEncMedia = 2554;
  marktest.MapTime = 3210;
  marktest.MapStatus = CAR_IN_LINE;
  latMarks->SetMarkDataReg(marktest,2);
    marktest.MapEncMedia = 4999;
  marktest.MapTime = 7470;
  marktest.MapStatus = CAR_IN_CURVE;
  latMarks->SetMarkDataReg(marktest,3); 
  /*for(int i = 0;i < 5; i++){
    marktest.MapEncMedia = (i*6000) + 2000;
    marktest.MapTime = i*120;
    if(i%2==0) marktest.MapStatus = CAR_IN_CURVE;
    else marktest.MapStatus = CAR_IN_LINE;
    latMarks->SetMarkDataReg(marktest,i); 
  }*/
  // Setup
  ESP_LOGD(TAG, "Task criada!");

  vTaskSuspend(xTaskCarStatus);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Matriz com dados de media encoders,linha do carrinho
  int32_t Manualmap[2][40]={{0,0,0,0,0},   // media
                            {0,0,0,0,0}}; // linha
  int32_t FinalMark = latMarks->getFinalMark(); // Media dos encoders da marcação final
  int32_t PlusPulses = 0; // Pulsos a mais para a parada
  int iloop=0;
  // Loop
  for (;;)
  {
    
    int32_t mediaEnc = (speed->getEncRight() + speed->getEncLeft())/2; // calcula media dos encoders
    if(iloop>=20 && DEBUGON)
    {
      ESP_LOGI(TAG, "CarStatus: %d", status->getState());
      ESP_LOGI(TAG, "EncMedia: %d", mediaEnc);
      ESP_LOGI(TAG, "FinalMark: %d", FinalMark);
      ESP_LOGI(TAG, "SetPointTrans: %d", PidTrans->getSetpoint());
      iloop=0;
    }
    iloop++;
    if (mediaEnc >= FinalMark + PlusPulses && !status->getMapping()) 
    {
      vTaskDelay(500/portTICK_PERIOD_MS);
      vTaskSuspend(xTaskPID);
      vTaskSuspend(xTaskSensors);
      braia->getStatus()->setState(CAR_STOPPED);
    }
    if(!status->getMapping() && mediaEnc < FinalMark + PlusPulses){ // define o status do carrinho se o mapeamento não estiver ocorrendo
      int mark = 0;
      for(mark=0; mark < Marks; mark++){ // Verifica a contagem do encoder e atribui o estado ao robô
        if(mark < Marks-1){
          int32_t Manualmedia = (latMarks->getMarkDataReg(mark)).MapEncMedia; //Manualmap[0][mark];
          int32_t ManualmediaNxt =  (latMarks->getMarkDataReg(mark+1)).MapEncMedia;//Manualmap[0][mark+1];
          if(mediaEnc >= Manualmedia && mediaEnc <= ManualmediaNxt){ // análise do valor das médias dos encoders
            int32_t mapstatus = (latMarks->getMarkDataReg(mark)).MapStatus; // status do robô
            CarState estado;
            if(mapstatus == CAR_IN_LINE) {
              estado = CAR_IN_LINE;
            } 
            else {
              estado = CAR_IN_CURVE;
            }
            status->setState(estado);
            break;
          }
        }
        else{
          int32_t mapstatus = (latMarks->getMarkDataReg(mark)).MapStatus; // status do robô
          CarState estado;
          if(mapstatus == CAR_IN_LINE) {
            estado = CAR_IN_LINE;
          }
          else {
            estado = CAR_IN_CURVE;
          }
          status->setState(estado);
          break;
        }
      }

    }

    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
  }
}

/* 
  * Task destinada a calcular velocidade instantanea e media dos motores
  * atraves dos valores de pulsos dos encoders 
  * */
void vTaskSpeed(void *pvParameters)
{
  // Tempo de delay
  auto const TaskDelay = 10; // 10 ms

  static const char *TAG = "vTaskSpeed";

  Robot *braia = (Robot *)pvParameters;
  dataSpeed *speed = braia->getSpeed();

  auto MPR_MotEsq = 600; // Variável com constante de conversão de pulsos para Rpm, Consertar o método que retorna esta variável
  auto MPR_MotDir = 600;

  // Componente de gerenciamento dos encoders
  ESP32Encoder enc_motEsq;
  ESP32Encoder enc_motDir;

  // GPIOs dos encoders dos encoders dos motores
  enc_motEsq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
  enc_motDir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);

  TickType_t lastTicksRevsCalc = 0;
  int32_t lastPulseRight = 0;
  int32_t lastPulseLeft = 0;
  uint16_t deltaTimeMS_inst; // delta entre ultimo calculo e o atual em millisegundos

  TickType_t initialTicksCar = 0;
  uint16_t deltaTimeMS_media;

  // Setup
  ESP_LOGD(TAG, "Task criada! TaskDelay: %d", TaskDelay);
  vTaskSuspend(xTaskSpeed);

  ESP_LOGD(TAG, "Retomada!");

  // Variavel necerraria para funcionaliade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // Variavel contendo quantidade de pulsos inicial do carro
  initialTicksCar = xTaskGetTickCount();

  // Quando for começar a utilizar, necessario limpeza da contagem.
  enc_motEsq.clearCount();
  enc_motDir.clearCount();
  // Loop
  for (;;)
  {
    deltaTimeMS_inst = (xTaskGetTickCount() - lastTicksRevsCalc) * portTICK_PERIOD_MS;
    lastTicksRevsCalc = xTaskGetTickCount();

    deltaTimeMS_media = (xTaskGetTickCount() - initialTicksCar) * portTICK_PERIOD_MS;
    // Calculos de velocidade instantanea (RPM)
    speed->setRPMLeft_inst(                         // -> Calculo velocidade instantanea motor esquerdo
        (((enc_motEsq.getCount() - lastPulseLeft)   // Delta de pulsos do encoder esquerdo
          / (float)MPR_MotEsq)          // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
         / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
         ));
    lastPulseLeft = enc_motEsq.getCount(); // Salva pulsos do encoder para ser usado no proximo calculo
    speed->setEncLeft(lastPulseLeft); //Salva pulsos do encoder esquerdo na classe speed

    speed->setRPMRight_inst(                        // -> Calculo velocidade instantanea motor direito
        (((enc_motDir.getCount() - lastPulseRight)  // Delta de pulsos do encoder esquerdo
          / (float)MPR_MotDir)          // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
         / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
         ));
    lastPulseRight = enc_motDir.getCount(); // Salva pulsos do motor para ser usado no proximo calculo
    speed->setEncRight(lastPulseRight); //Salva pulsos do encoder direito na classe speed

    // Calculo de velocidade media do carro (RPM)
    speed->setRPMCar_media(                                                                                      // -> Calculo velocidade media do carro
        (((lastPulseRight / (float)MPR_MotDir + lastPulseLeft / (float)MPR_MotEsq)) / 2) // Revolucoes media desde inicializacao
        / ((float)deltaTimeMS_media / (float)60000)                                                              // Divisao do delta tempo em minutos para calculo de RPM
    );
    //ESP_LOGE(TAG,"Direito: %d",enc_motDir.getCount());
    //ESP_LOGE(TAG,"Direito: %d",enc_motEsq.getCount());
    //ESP_LOGD("vTaskSpeed", "encDir: %d | encEsq: %d", enc_motDir.getCount(), enc_motEsq.getCount());
    //ESP_LOGD("vTaskSpeed", "VelEncDir: %d | VelEncEsq: %d", speed->getRPMRight_inst(), speed->getRPMLeft_inst());

    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
  }
}

void vTaskMapping(void *pvParameters){

  //setup
  //gpio_pad_select_gpio(17);
  //gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
  //gpio_pad_select_gpio(05);
  //gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
  bool DEBUGON = true;
  gpio_pad_select_gpio(0);
  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
  gpio_set_pull_mode(GPIO_NUM_0,GPIO_PULLUP_ONLY);

  
  static const char *TAG = "vTaskMapping";
  ESP_LOGI(TAG, "Mapeamento Iniciado e aguardando calibração");
  vTaskSuspend(xTaskMapping);
  ESP_LOGI(TAG, "Mapeamento Retomado!");
  

  Robot *braia = (Robot *)pvParameters;
  auto speedMapping = braia->getSpeed();
  auto SLat = braia->getsLat();
  auto latMarks = braia->getSLatMarks();
  
  //speedMapping -> setSpeedMin(50, CAR_IN_LINE);//velocidade minima de mapeamento e estado do robô(linha)
  //speedMapping -> setSpeedMax(70, CAR_IN_LINE);//velocidade maxima de mapeamento e estado do robô(linha)
  //speedMapping -> setSpeedBase(((50+70)/2), CAR_IN_LINE);//velocidade base de mapeamento e estado do robô(linha)

  //Inicio mapeamento
  int32_t mappingData[3][40]={{0,0,0},{0,0,0},{0,0,0}}; // [tempo][media][estado]
                    //  "quantidade de marcações"

  //mappingData[2][L,L,R, , , , , , , , , , , , , , , ,]; considerando o tempo do video, adicionar os estados do robô
  uint16_t marks = 0;

  TickType_t xLastWakeTime = xTaskGetTickCount();// Variavel necesárraria para funcionalidade do vTaskDelayUtil, quarda a contagem de pulsos da CPU
  struct MapData markreg;
  markreg.MapEncMedia = 0;
  markreg.MapTime = 0;
  markreg.MapStatus = CAR_IN_LINE;
  latMarks->SetMarkDataReg(markreg,0);
  bool startTimer = false;
  bool mapfinish = false;
  bool leftpassed = false;
  int iloop=0; // Variável para debug
  int32_t FinalMarkData = 0; // Media dos encoders na marcação final
  int32_t InitialMarkData = 0; // Media dos encoders na marcação inicial

  TickType_t xInicialTicks = xTaskGetTickCount();

  // Loop
  for (;;)
  {
    
    uint16_t slesq1 = SLat->getChannel(0);
    uint16_t slesq2 = SLat->getChannel(1);
    //bool sldir1 = gpio_get_level(GPIO_NUM_17);
    bool sldir2 = gpio_get_level(GPIO_NUM_5);
    bool bottom = gpio_get_level(GPIO_NUM_0);
    
    if(((latMarks->getrightMarks()) == 1) && !startTimer){
      
      xInicialTicks = xTaskGetTickCount(); //pegando o tempo inicial
      startTimer = true;
      InitialMarkData = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
      latMarks->SetInitialMark(InitialMarkData);
    }
    if(iloop>=20 && DEBUGON)
    {
      ESP_LOGI("vTaskMapping", "Laterais Esquerdos: %d | %d ", slesq1 , slesq2);
      ESP_LOGI("vTaskMapping", "Laterais Direitos: %d", sldir2);
      ESP_LOGI(TAG, "Marcações direita: %d ", latMarks->getrightMarks());
      iloop=0;
    }
    iloop++;
    if((latMarks->getrightMarks()) == 1){
      if ((slesq1 < 300) && (sldir2) && !leftpassed)
      {
        struct MapData MarkReg;
        //tempo
        MarkReg.MapTime = (xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS;
        //media
        MarkReg.MapEncMedia = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
        //estado
        if((marks % 2) == 0){ // Verifica se o carrinho está na curva ou na linha
          MarkReg.MapStatus = CAR_IN_CURVE;
        }
        else{
          MarkReg.MapStatus = CAR_IN_LINE;
        }
        ESP_LOGI(TAG,"%d, %d, %d", MarkReg.MapTime,MarkReg.MapEncMedia, MarkReg.MapStatus);
        latMarks->SetMarkDataReg(MarkReg, marks+1); // Salva os dados da marcação na struct MapData
        marks ++;
        leftpassed = true; // Diz que o carro está em uma marcação esquerda

      }
      else if (slesq1 > 600){
        leftpassed = false;
      } 

    }
    else if(latMarks->getrightMarks() < 1){
      //ESP_LOGI(TAG, "Mapeamento não iniciado");
    }
    else if(latMarks->getrightMarks() > 1 && !mapfinish){
      //ESP_LOGI(TAG, "Mapeamento finalizado");
      FinalMarkData = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
      latMarks->SetMapFinished(true);
      latMarks->SetFinalMark(FinalMarkData);
      latMarks->SetTotalLeftMarks(marks);
      mapfinish = true;
    }
    if(!bottom){
      ESP_LOGI("","Tempo, Média, Estado");
      for(int i = 0; i < marks+1;  i++){
         struct MapData markreg = latMarks->getMarkDataReg(i); 
         ESP_LOGI("","%d, %d, %d", markreg.MapTime,markreg.MapEncMedia, markreg.MapStatus);
      }
      ESP_LOGI("Initial Mark (média dos encoders) "," %d ", latMarks->getInitialMark());
      ESP_LOGI("Final Mark (média dos encoders) "," %d ", latMarks->getFinalMark());
      ESP_LOGI("Total de marcações esquerdas "," %d ", latMarks->getTotalLeftMarks());
    }

    vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);  
  }         
}

void vTaskEspNow(void *pvParameters){
  
  auto TaskDelay = 30;
  static const char *TAG = "vTaskEspNow";
  ESP_LOGD(TAG, "Task EspNow Iniciada e aguardando calibração");
  vTaskSuspend(xTaskEspNow);
  ESP_LOGD(TAG, "TaskEspNow Retomada!");
  
  uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xB2, 0x23, 0xD0};
  Robot *braia = (Robot *)pvParameters;
  EspNowHandler protocolHandler;
  protocolHandler.EspNowInit(1,broadcastAddress,false);
  char msgSend[] = "Mensagem do Robo";
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for(;;){
    uint8_t *ReceivedData;
    if(protocolHandler.dataAvailable()){
      ReceivedData = protocolHandler.getReceivedData();

    }

    protocolHandler.EspSend(MapDataSend,0,0,msgSend);
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
  
  braia->getStatus()->setMapping(true);
  bool mapping = braia->getStatus()->getMapping();
  braia->getStatus()->setState(CAR_IN_LINE);

  //Pulsos para uma revolução de cada encoder (revolução*redução)
  // braia->getSpeed()->setMPR_MotDir(20,30);
  // braia->getSpeed()->setMPR_MotEsq(20,30);

  if(mapping){

    braia->getSpeed()->setSpeedBase(25, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(25, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(50, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.000, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.000, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.27, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(100);

  }
  else{

    braia->getSpeed()->setSpeedBase(40, CAR_IN_LINE);
    braia->getSpeed()->setSpeedBase(20, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMax(70, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMax(50, CAR_IN_CURVE);

    braia->getSpeed()->setSpeedMin(5, CAR_IN_LINE);
    braia->getSpeed()->setSpeedMin(5, CAR_IN_CURVE);

    braia->getPIDRot()->setKd(0.0025, CAR_IN_LINE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_LINE);
    braia->getPIDRot()->setKd(0.0025, CAR_IN_CURVE);
    braia->getPIDVel()->setKd(0.0, CAR_IN_CURVE);

    braia->getPIDRot()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_LINE);
    braia->getPIDRot()->setKi(0.00, CAR_IN_CURVE);
    braia->getPIDVel()->setKi(0.00, CAR_IN_CURVE);

    braia->getPIDRot()->setKp(0.27, CAR_IN_LINE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_LINE);
    braia->getPIDRot()->setKp(0.27, CAR_IN_CURVE);
    braia->getPIDVel()->setKp(0.035, CAR_IN_CURVE);

    braia->getPIDVel()->setSetpoint(1500);

  }
  
  // Criacao das tasks e definindo seus parametros
  //xTaskCreate(FUNCAO, NOME, TAMANHO DA HEAP, ARGUMENTO, PRIORIDADE, TASK HANDLE)
  ESP_LOGI("TaskMotors", "TaskMotors Rodando");
  xTaskCreate(vTaskMotors, "TaskMotors", 10000, braia, 9, &xTaskMotors);
  ESP_LOGI("TaskSensors", "TaskSensors Rodando");
  xTaskCreate(vTaskSensors, "TaskSensors", 10000, braia, 9, &xTaskSensors);
  ESP_LOGI("TaskPid", "TaskPid Rodando");
  xTaskCreate(vTaskPID, "TaskPID", 10000, braia, 9, &xTaskPID);
  ESP_LOGI("TaskSpeed", "TaskSpeed Rodando");
  xTaskCreate(vTaskSpeed, "TaskSpeed", 10000, braia, 9, &xTaskSpeed);
  ESP_LOGI("TaskCarStatus", "TaskCarStatus Rodando");
  xTaskCreate(vTaskCarStatus, "TaskCarStatus", 10000, braia, 9, &xTaskCarStatus);
  ESP_LOGI("TaskMap", "TaskMap Rodando");
  xTaskCreate(vTaskMapping, "TaskMapping", 10000, braia, 9, &xTaskMapping);
  //ESP_LOGI("TaskEspNow", "TaskEspNow Rodando");
  //xTaskCreate(vTaskEspNow, "TaskEspNow", 10000, braia, 9, &xTaskEspNow);

}