#include "MappingService.h"

void MappingService::Main(){
//   //setup
//   //gpio_pad_select_gpio(17);
//   //gpio_set_direction(GPIO_NUM_17, GPIO_MODE_INPUT);
//   //gpio_pad_select_gpio(05);
//   //gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
//   gpio_pad_select_gpio(0);
//   gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
//   gpio_set_pull_mode(GPIO_NUM_0,GPIO_PULLUP_ONLY);

  
//   static const char *TAG = "vTaskMapping";
//   ESP_LOGD(TAG, "Mapeamento Iniciado e aguardando calibração");
//   vTaskSuspend(xTaskMapping);
//   ESP_LOGD(TAG, "Mapeamento Retomado!");
  

//   Robot *braia = (Robot *)pvParameters;
//   auto speedMapping = braia->getSpeed();
  
//   //speedMapping -> setSpeedMin(50, CAR_IN_LINE);//velocidade minima de mapeamento e estado do robô(linha)
//   //speedMapping -> setSpeedMax(70, CAR_IN_LINE);//velocidade maxima de mapeamento e estado do robô(linha)
//   //speedMapping -> setSpeedBase(((50+70)/2), CAR_IN_LINE);//velocidade base de mapeamento e estado do robô(linha)

//   //Inicio mapeamento
//   int32_t mappingData[3][40]={{0,0,0},{0,0,0},{0,0,0}}; // [tempo][media][estado]
//                     //  "quantidade de marcações"

//   //mappingData[2][L,L,R, , , , , , , , , , , , , , , ,]; considerando o tempo do video, adicionar os estados do robô
//   uint16_t marks = 0;

//   TickType_t xLastWakeTime = xTaskGetTickCount();// Variavel necerraria para funcionalidade do vTaskDelayUtil, quarda a contagem de pulsos da CPU

//   bool startTimer = false;
//   int32_t FinalMarkData = 0; // Media dos encoders na marcação final

//   TickType_t xInicialTicks = xTaskGetTickCount();

//   // Loop
//   for (;;)
//   {
    
//     auto SLat = braia->getsLat();
//     uint16_t slesq1 = SLat->getChannel(0);
//     uint16_t slesq2 = SLat->getChannel(1);
//     bool sldir1 = gpio_get_level(GPIO_NUM_17);
//     bool sldir2 = gpio_get_level(GPIO_NUM_5);
//     auto latMarks = braia->getSLatMarks();
//     bool bottom = gpio_get_level(GPIO_NUM_0);
    
//     if(((latMarks->getrightMarks()) == 1) && !startTimer){
      
//       xInicialTicks = xTaskGetTickCount(); //pegando o tempo inicial
//       startTimer = true;
//     }


//     if((latMarks->getrightMarks()) == 1){

//       if ((slesq1 < 300 || slesq2 < 300) && (sldir1 && sldir2))
//       {
//         //tempo
//         mappingData[0][marks] = (xTaskGetTickCount() - xInicialTicks) * portTICK_PERIOD_MS;
//         //media
//         mappingData[1][marks] = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
//         //estado
//         marks ++;

//       } 

//     }
//     else if(latMarks->getrightMarks() < 1){
//       ESP_LOGD(TAG, "Mapeamento não iniciado");
//     }
//     else if(latMarks->getrightMarks() > 1){
//       ESP_LOGD(TAG, "Mapeamento finalizado");
//       FinalMarkData = ((speedMapping->getEncRight()) + (speedMapping->getEncLeft())) / 2;
//     }
//     if(!bottom){
//       ESP_LOGD("","Tempo, Média, Estado");
//       for(int i = 0; i < marks;  i++){
//          ESP_LOGD("","%d, %d, %d", mappingData[0][i],mappingData[1][i], mappingData[2][i]);
//       }
//       ESP_LOGD("Final Mark (média dos encoders) "," %d ", FinalMarkData);
//     }

//     vTaskDelayUntil(&xLastWakeTime, 30 / portTICK_PERIOD_MS);  
//   }         
}