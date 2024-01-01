#include "SpeedService.hpp"

SpeedService::SpeedService(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{
    this->robot = Robot::getInstance();
    this->speed = robot->getSpeed();

    this->diameterWheel = speed->WheelDiameter->getData(); 
    this->diameterRobot = speed->RobotDiameter->getData();

    // GPIOs dos encoders dos encoders dos motores
    //enc_motEsq.attachFullQuad(ENC_MOT_ESQ_B, ENC_MOT_ESQ_A);
    //enc_motDir.attachFullQuad(ENC_MOT_DIR_B, ENC_MOT_DIR_A);

    enc_motEsq.attachFullQuad(ENC_MOT_ESQ_A, ENC_MOT_ESQ_B);
    enc_motDir.attachFullQuad(ENC_MOT_DIR_A, ENC_MOT_DIR_B);

    MPR_Mot = speed->MPR->getData();
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
        vTaskDelayUntil(&xLastWakeTime, TaskDelay / portTICK_PERIOD_MS);

        estado = (CarState)robot->getStatus()->robotState->getData();

        if (estado == CAR_STOPPED && robot->getSLatMarks()->rightMarks->getData() == 0)
        {
            enc_motEsq.clearCount();
            enc_motDir.clearCount();
            lastPulseLeft = 0;
            lastPulseRight = 0;
        }

        deltaTimeMS_inst = (xTaskGetTickCount() - lastTicksRevsCalc) * portTICK_PERIOD_MS;
        lastTicksRevsCalc = xTaskGetTickCount();

        deltaTimeMS_media = (xTaskGetTickCount() - initialTicksCar) * portTICK_PERIOD_MS;

        deltaEncDir=(enc_motDir.getCount() - lastPulseRight);
        deltaEncEsq=(enc_motEsq.getCount() - lastPulseLeft);

        deltaS = ((deltaEncDir+deltaEncEsq) * M_PI * diameterWheel )/((float)2.0*MPR_Mot);
        deltaA = ((deltaEncEsq-deltaEncDir) * M_PI * diameterWheel )/((float)MPR_Mot*diameterRobot); 


        if (estado != CAR_STOPPED)
        {
            Ang += deltaA;
            DeltaPositionX = abs(deltaS) * cos(Ang);
            DeltaPositionY = abs(deltaS) * sin(Ang);
            positionX += DeltaPositionX;
            positionY += DeltaPositionY; 
            speed->positionX->setData(positionX / 10.0); // cm
            speed->positionY->setData(positionY / 10.0); // cm
        }

        // Calculos de velocidade instantanea (RPM)
        speed->RPMLeft_inst->setData(                   // -> Calculo velocidade instantanea motor esquerdo
            (((enc_motEsq.getCount() - lastPulseLeft)   // Delta de pulsos do encoder esquerdo
              / (float)MPR_Mot)                      // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
             / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
             ));
        lastPulseLeft = enc_motEsq.getCount();  // Salva pulsos do encoder para ser usado no proximo calculo
        speed->EncLeft->setData(lastPulseLeft); // Salva pulsos do encoder esquerdo na classe speed

        speed->RPMRight_inst->setData(                  // -> Calculo velocidade instantanea motor direito
            (((enc_motDir.getCount() - lastPulseRight)  // Delta de pulsos do encoder esquerdo
              / (float)MPR_Mot)                      // Conversao para revolucoes de acordo com caixa de reducao e pulsos/rev
             / ((float)deltaTimeMS_inst / (float)60000) // Divisao do delta tempo em minutos para calculo de RPM
             ));
        lastPulseRight = enc_motDir.getCount();   // Salva pulsos do motor para ser usado no proximo calculo
        speed->EncRight->setData(lastPulseRight); // Salva pulsos do encoder direito na classe speed

        speed->EncMedia->setData((lastPulseLeft + lastPulseRight)/2);
        // Calculo de velocidade media do carro (RPM)
        speed->RPMCar_media->setData(                                                                              // -> Calculo velocidade media do carro
            (((lastPulseRight / (float)speed->MPR->getData() + lastPulseLeft / (float)speed->MPR->getData())) / 2) // Revolucoes media desde inicializacao
            / ((float)deltaTimeMS_media / (float)60000)                                                            // Divisao do delta tempo em minutos para calculo de RPM
        );

        if (iloop >= 100)
        {
            ESP_LOGD(GetName().c_str(), "encDir: %ld | encEsq: %ld | encmedia: %ld", enc_motDir.getCount(), enc_motEsq.getCount(), speed->EncMedia->getData());
            ESP_LOGD(GetName().c_str(), "deltaX: %.4f | deltaY: %.4f |  PositionX: %.4f | PositionY: %.4f", DeltaPositionX, DeltaPositionY, positionX,positionY);
            ESP_LOGD(GetName().c_str(), "Soma: %d - VelEncDir: %d | VelEncEsq: %d", (speed->RPMRight_inst->getData() + speed->RPMLeft_inst->getData()), speed->RPMRight_inst->getData(), speed->RPMLeft_inst->getData());
            ESP_LOGD(GetName().c_str(), "MPR: %d",speed->MPR->getData());
            iloop = 0;
        }
        iloop++;
    }
}