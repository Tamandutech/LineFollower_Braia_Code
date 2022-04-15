#include "SpeedService.hpp"

SpeedService::SpeedService(const char *name, Robot *robot, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = robot;
    this->speed = robot->getSpeed();

    // GPIOs dos encoders dos encoders dos motores
    enc_motEsq.attachFullQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
    enc_motDir.attachFullQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);
};

void SpeedService::Run()
{
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
        CarState estado = (CarState)robot->getStatus()->robotState->getData();

        if (estado == CAR_STOPPED && robot->getSLatMarks()->rightMarks == 0)
        {
            enc_motEsq.clearCount();
            enc_motDir.clearCount();
            lastPulseLeft = 0;
            lastPulseRight = 0;
        }

        deltaTimeMS_inst = (xTaskGetTickCount() - lastTicksRevsCalc) * portTICK_PERIOD_MS;
        lastTicksRevsCalc = xTaskGetTickCount();

        deltaTimeMS_media = (xTaskGetTickCount() - initialTicksCar) * portTICK_PERIOD_MS;
        // Calculos de velocidade instantanea (RPM)
        speed->RPMLeft_inst->setData(                   // -> Calculo velocidade instantanea motor esquerdo
            (((enc_motEsq.getCount() - lastPulseLeft)   // Delta de pulsos do encoder esquerdo
              / (float)MPR_MotEsq)                      // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
             / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
             ));
        lastPulseLeft = enc_motEsq.getCount();  // Salva pulsos do encoder para ser usado no proximo calculo
        speed->EncLeft->setData(lastPulseLeft); // Salva pulsos do encoder esquerdo na classe speed

        speed->RPMRight_inst->setData(                  // -> Calculo velocidade instantanea motor direito
            (((enc_motDir.getCount() - lastPulseRight)  // Delta de pulsos do encoder esquerdo
              / (float)MPR_MotDir)                      // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
             / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
             ));
        lastPulseRight = enc_motDir.getCount();   // Salva pulsos do motor para ser usado no proximo calculo
        speed->EncRight->setData(lastPulseRight); // Salva pulsos do encoder direito na classe speed

        // Calculo de velocidade media do carro (RPM)
        speed->RPMCar_media->setData(                                                                                            // -> Calculo velocidade media do carro
            (((lastPulseRight / (float)speed->MPR_MotDir->getData() + lastPulseLeft / (float)speed->MPR_MotEsq->getData())) / 2) // Revolucoes media desde inicializacao
            / ((float)deltaTimeMS_media / (float)60000)                                                                          // Divisao do delta tempo em minutos para calculo de RPM
        );
        if (iloop >= 200)
        {
            ESP_LOGD(GetName().c_str(), "Direito: %d", enc_motDir.getCount());
            ESP_LOGD(GetName().c_str(), "Direito: %d", enc_motEsq.getCount());
            ESP_LOGD(GetName().c_str(), "encDir: %d | encEsq: %d", enc_motDir.getCount(), enc_motEsq.getCount());
            ESP_LOGD(GetName().c_str(), "VelEncDir: %d | VelEncEsq: %d", speed->RPMRight_inst->getData(), speed->RPMLeft_inst->getData());
            iloop = 0;
        }
        iloop++;
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);
    }
}