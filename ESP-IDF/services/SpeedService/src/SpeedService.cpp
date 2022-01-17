#include "SpeedService.h"

void SpeedService::Main(){
//   // Tempo de delay
//   auto const TaskDelay = 10; // 10 ms

//   static const char *TAG = "vTaskSpeed";

//   Robot *braia = (Robot *)pvParameters;
//   dataSpeed *speed = braia->getSpeed();

//   auto MPR_MotEsq = 600;
//   auto MPR_MotDir = 600;

//   // Componente de gerenciamento dos encoders
//   ESP32Encoder enc_motEsq;
//   ESP32Encoder enc_motDir;

//   // GPIOs dos encoders dos encoders dos motores
//   enc_motEsq.attachHalfQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
//   enc_motDir.attachHalfQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);

//   TickType_t lastTicksRevsCalc = 0;
//   int32_t lastPulseRight = 0;
//   int32_t lastPulseLeft = 0;
//   uint16_t deltaTimeMS_inst; // delta entre ultimo calculo e o atual em millisegundos

//   TickType_t initialTicksCar = 0;
//   uint16_t deltaTimeMS_media;

//   // Setup
//   ESP_LOGD(TAG, "Task criada! TaskDelay: %d", TaskDelay);
//   vTaskSuspend(xTaskSpeed);

//   ESP_LOGD(TAG, "Retomada!");

//   // Variavel necerraria para funcionaliade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   // Variavel contendo quantidade de pulsos inicial do carro
//   initialTicksCar = xTaskGetTickCount();

//   // Quando for começar a utilizar, necessario limpeza da contagem.
//   enc_motEsq.clearCount();
//   enc_motDir.clearCount();
//   // Loop
//   for (;;)
//   {
//     deltaTimeMS_inst = (xTaskGetTickCount() - lastTicksRevsCalc) * portTICK_PERIOD_MS;
//     lastTicksRevsCalc = xTaskGetTickCount();

//     deltaTimeMS_media = (xTaskGetTickCount() - initialTicksCar) * portTICK_PERIOD_MS;
//     // Calculos de velocidade instantanea (RPM)
//     speed->setRPMLeft_inst(                         // -> Calculo velocidade instantanea motor esquerdo
//         (((enc_motEsq.getCount() - lastPulseLeft)   // Delta de pulsos do encoder esquerdo
//           / (float)MPR_MotEsq)          // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
//          / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
//          ));
//     lastPulseLeft = enc_motEsq.getCount(); // Salva pulsos do encoder para ser usado no proximo calculo
//     speed->setEncLeft(lastPulseLeft); //Salva pulsos do encoder esquerdo na classe speed

//     speed->setRPMRight_inst(                        // -> Calculo velocidade instantanea motor direito
//         (((enc_motDir.getCount() - lastPulseRight)  // Delta de pulsos do encoder esquerdo
//           / (float)MPR_MotDir)          // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
//          / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
//          ));
//     lastPulseRight = enc_motDir.getCount(); // Salva pulsos do motor para ser usado no proximo calculo
//     speed->setEncRight(lastPulseRight); //Salva pulsos do encoder direito na classe speed

//     // Calculo de velocidade media do carro (RPM)
//     speed->setRPMCar_media(                                                                                      // -> Calculo velocidade media do carro
//         (((lastPulseRight / (float)speed->getMPR_MotDir() + lastPulseLeft / (float)speed->getMPR_MotEsq())) / 2) // Revolucoes media desde inicializacao
//         / ((float)deltaTimeMS_media / (float)60000)                                                              // Divisao do delta tempo em minutos para calculo de RPM
//     );
//     //ESP_LOGE(TAG,"Direito: %d",enc_motDir.getCount());
//     //ESP_LOGE(TAG,"Direito: %d",enc_motEsq.getCount());
//     //ESP_LOGD("vTaskSpeed", "encDir: %d | encEsq: %d", enc_motDir.getCount(), enc_motEsq.getCount());
//     //ESP_LOGD("vTaskSpeed", "VelEncDir: %d | VelEncEsq: %d", speed->getRPMRight_inst(), speed->getRPMLeft_inst());

//     xLastWakeTime = xTaskGetTickCount();
//     vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
//   }
}