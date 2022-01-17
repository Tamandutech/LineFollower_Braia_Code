#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include "Service.h"

// #include "io.h"

// #include "QTRSensors.h"

class SensorsService : Service
{
public:
    SensorsService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Service (name, stackDepth, priority){};
    ~SensorsService();

    void Main() override;

private:
// void calibAllsensors(QTRSensors *sArray, QTRSensors *SLat, Robot *braia)
// {
//   //Calibração dos dos sensores laterais e array
//   for (uint16_t i = 0; i < 200; i++)
//   {
//     sArray->calibrate();
//     SLat->calibrate();
//     vTaskDelay(20 / portTICK_PERIOD_MS);
//   }
//   //leitura e armazenamento dos valores máximos e mínimos dos sensores obtidos na calibração
//   std::vector<uint16_t> sArrayMaxes(sArray->calibrationOn.maximum, sArray->calibrationOn.maximum + sArray->getSensorCount());
//   std::vector<uint16_t> sArrayMins(sArray->calibrationOn.minimum, sArray->calibrationOn.minimum + sArray->getSensorCount());
//   std::vector<uint16_t> SLatMaxes(SLat->calibrationOn.maximum, SLat->calibrationOn.maximum + SLat->getSensorCount());
//   std::vector<uint16_t> SLatMins(SLat->calibrationOn.minimum, SLat->calibrationOn.minimum + SLat->getSensorCount());

//   //armazenamento dos valores máximos e mínimos dos sensores no objeto Braia
//   braia->getsArray()->setChannelsMaxes(sArrayMaxes);
//   braia->getsArray()->setChannelsMins(sArrayMins);
//   braia->getsLat()->setChannelsMaxes(SLatMaxes);
//   braia->getsLat()->setChannelsMins(SLatMins);
// }

// void getSensors(QTRSensors *sArray, QTRSensors *SLat, Robot *braia) // função leitura dos sensores
// {
//   //Arrays para armazenar leitura bruta dos sensores array e laterais
//   uint16_t sArraychannels[sArray->getSensorCount()];
//   uint16_t SLatchannels[SLat->getSensorCount()];

// #ifdef LINE_COLOR_BLACK
//   braia->getsArray()->setLine(sArray->readLineBlack(sArraychannels));  
// #else
//   braia->getsArray()->setLine(sArray->readLineWhite(sArraychannels));
// #endif
//                                // cálculo dos valores do sensor array
//   SLat->readCalibrated(SLatchannels);                                                                 //leitura dos sensores laterais
//   std::vector<uint16_t> sArraychannelsVec(sArraychannels, sArraychannels + sArray->getSensorCount()); // vector(array) com os valores do sensor array
//   std::vector<uint16_t> SLatchannelsVec(SLatchannels, SLatchannels + SLat->getSensorCount());         // vector(array) com os valores dos sensores laterais

//   //armazenando da leitura bruta do sensor array e lateral no objeto Braia
//   braia->getsArray()->setChannels(sArraychannelsVec);
//   braia->getsLat()->setChannels(SLatchannelsVec);

//   ESP_LOGD("getSensors", "Array: %d | %d | %d | %d | %d | %d | %d | %d ", sArraychannels[0], sArraychannels[1], sArraychannels[2], sArraychannels[3], sArraychannels[4], sArraychannels[5], sArraychannels[6], sArraychannels[7]);
//   ESP_LOGD("getSensors", "Linha: %d", braia->getsArray()->getLine());
//   ESP_LOGD("getSensors", "Laterais: %d | %d ", SLatchannels[0], SLatchannels[1]);

//   //braia->getsLat()->setLine((SLatchannels[0]+SLatchannels[1])/2-(SLatchannels[2]+SLatchannels[3])/2); // cálculo dos valores dos sensores laterais
// }

// void processSLat(Robot *braia)
// {
//   bool sldir1 = gpio_get_level(GPIO_NUM_17);
//   bool sldir2 = gpio_get_level(GPIO_NUM_5);

//   ESP_LOGD("processSLat", "Laterais (Direira): %d | %d", sldir1, sldir2);

//   auto SLat = braia->getsLat();
//   uint16_t slesq1 = SLat->getChannel(0);
//   uint16_t slesq2 = SLat->getChannel(1);
//   auto latMarks = braia->getSLatMarks();
//   if (slesq1 < 300 || slesq2 < 300 || !sldir1 || !sldir2) // leitura de faixas brancas sensores laterais
//   {
//     if ((slesq1 < 300 || slesq2 < 300) && (sldir1 && sldir2)) //lendo sLat esq. branco e dir. preto
//     {
//       if (!(latMarks->getSLatEsq()))
//         latMarks->leftPassedInc();
//       latMarks->SetSLatEsq(true);
//       latMarks->SetSLatDir(false);
//     }
//     else if ((!sldir1 || !sldir2) && (slesq1 > 600 && slesq2 > 600)) // lendo sldir. branco e sLat esq. preto 
//     {
//       if (!(latMarks->getSLatDir()))
//         latMarks->rightPassedInc();
//       latMarks->SetSLatDir(true);
//       latMarks->SetSLatEsq(false);
//     }
//   }
//   else
//   {
//     latMarks->SetSLatDir(false);
//     latMarks->SetSLatEsq(false);
//   }

//   if(slesq1 < 300 && slesq2 < 300 && !sldir1 && !sldir2){//continuar em frente em intersecção de linhas
//     braia->getStatus()->setState(CAR_IN_LINE);
//   }

//   if(latMarks->getrightMarks() >= 2){ //parar depois da leitura da segunda linha direita
//     vTaskDelay(500);
//     braia->getStatus()->setState(CAR_STOPPED);
//   }
// }


};

#endif