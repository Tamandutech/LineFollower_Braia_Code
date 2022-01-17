#include "CarStatusService.h"

void CarStatusService::Main(){
//   static const char *TAG = "vTaskCarStatus";
//   #define Marks 40 // marcas laterais esquerda na pista 

//   Robot *braia = (Robot *)pvParameters;
//   RobotStatus *status = braia->getStatus();
//   dataSpeed *speed = braia->getSpeed();
  
//   // Setup
//   ESP_LOGD(TAG, "Task criada!");

//   vTaskSuspend(xTaskCarStatus);

//   ESP_LOGD(TAG, "Retomada!");

//   // Variavel necerraria para funcionalidade do vTaskDelayUtil, guarda a contagem de pulsos da CPU
//   TickType_t xLastWakeTime = xTaskGetTickCount();

//   // Matriz com dados de media encoders,linha do carrinho
//   int32_t Manualmap[2][40]={{0,0,0,0,0},   // media
//                             {0,0,0,0,0}}; // linha
//   int32_t FinalMark = 0; // Media dos encoders da marcação final
//   int32_t PlusPulses = 0; // Pulsos a mais para a parada
//   // Loop
//   for (;;)
//   {
//     ESP_LOGD(TAG, "CarStatus: %d", status->getState());
//     int32_t mediaEnc = (speed->getEncRight() + speed->getEncLeft())/2; // calcula media dos encoders
//     if (mediaEnc >= FinalMark + PlusPulses)  braia->getStatus()->setState(CAR_STOPPED);
//     if(!status->getMapping() && braia->getSLatMarks()->getrightMarks() < 2 && mediaEnc < FinalMark + PlusPulses){ // define o status do carrinho se o mapeamento não estiver ocorrendo
//       int mark = 0;
//       for(mark=0; mark < Marks; mark++){ // Verifica a contagem do encoder e atribui o estado ao robô
//         if(mark < Marks-1){
//           int32_t Manualmedia = Manualmap[0][mark];
//           int32_t ManualmediaNxt = Manualmap[0][mark+1];
//           if(mediaEnc >= Manualmedia && mediaEnc <= ManualmediaNxt){ // análise do valor das médias dos encoders
//             CarState estado;
//             if(Manualmap[1][mark] == CAR_IN_LINE) estado = CAR_IN_LINE; 
//             else estado = CAR_IN_CURVE;
//             status->setState(estado);
//             break;
//           }
//         }
//         else{
//           CarState estado;
//           if(Manualmap[1][mark] == CAR_IN_LINE) estado = CAR_IN_LINE;
//           else estado = CAR_IN_CURVE;
//           status->setState(estado);
//           break;
//         }
//       }

//     }

//     xLastWakeTime = xTaskGetTickCount();
//     vTaskDelayUntil(&xLastWakeTime, 100 / portTICK_PERIOD_MS);
//   }
}