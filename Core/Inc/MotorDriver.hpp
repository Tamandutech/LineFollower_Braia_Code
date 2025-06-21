#include <algorithm>

#include "stm32g4xx_hal.h"

typedef struct {
    // pino de direção do motor
    GPIO_TypeDef *dirPort;
    uint16_t dirPin;
    // pino de PWM do motor
    TIM_HandleTypeDef *pwmhtim;
    uint32_t pwmChannel;
    // timer do encoder
    TIM_HandleTypeDef *encoderhtim;
} MotorPins_t;

class MotorDriver {
   private:
    MotorPins_t motorPins;

   public:
    MotorDriver(MotorPins_t pins) {
        motorPins = pins;
        HAL_GPIO_WritePin(motorPins.dirPort, motorPins.dirPin, GPIO_PIN_RESET);
        HAL_TIM_PWM_Start(motorPins.pwmhtim, motorPins.pwmChannel);
        HAL_TIM_Encoder_Start(motorPins.encoderhtim, TIM_CHANNEL_ALL);
    }

    void pwmOutput(int16_t duty) {
        if (duty > 0) {
            HAL_GPIO_WritePin(motorPins.dirPort, motorPins.dirPin, GPIO_PIN_SET);  // Define a direção
        } else {
            HAL_GPIO_WritePin(motorPins.dirPort, motorPins.dirPin, GPIO_PIN_RESET);  // Inverte a direção
            duty = -duty;                                                            // Torna o duty positivo
        }
        duty = std::min(duty, (int16_t)1000);  // Garante que o duty está dentro do intervalo permitido
        __HAL_TIM_SET_COMPARE(motorPins.pwmhtim, motorPins.pwmChannel, duty);
    }

    int32_t getCounter() {
        return (int32_t)__HAL_TIM_GET_COUNTER(motorPins.encoderhtim);
    }

    void setCounter(int32_t value) {
        __HAL_TIM_SET_COUNTER(motorPins.encoderhtim, value);
    }
};