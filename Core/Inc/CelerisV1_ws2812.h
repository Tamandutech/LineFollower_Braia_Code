#ifndef LED_WS2812_H
#define LED_WS2812_H

#include <stdint.h>

#include "stm32g4xx_hal.h"

extern TIM_HandleTypeDef htim1;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

// Function write rgb color to the led strip
void setLedsColor(rgb_color_t* colors, uint8_t ledsCount) {
    /*
    5 * 8 pois o pwm esta configurado em um canal N em uma velocidade 5x mais rapida que o byte,
    então para cada bit do byte, são necessários 5 bits de pwm

    em canal N, o sinal é invertido, ou seja valores altos são 0 e valores baixos são 1

    um bit 0 para o led traduzido para o vetor é:
    | poisição | 0 | 1 | 2 | 3 | 4 |
    | valor    | 0 | 0 | 4 | 4 | 4 |
    um bit 1 para o led traduzido para o vetor é:
    | poisição | 0 | 1 | 2 | 3 | 4 |
    | valor    | 0 | 0 | 0 | 4 | 4 |
    */
    // o numero entre parenteses é o numero de leds, 4 é o numero maximo de leds que o buffer suporta
    static uint16_t buffer[5 * 8 * 3 * (4) + 200] = {0};  // 200 é o tempo de reset do sinal, para garantir que o sinal seja lido corretamente
    for (uint16_t i = 0; i < sizeof(buffer) / 2; i++) {
        buffer[i] = 4;  // inicializa o buffer com 4, que é o valor de low no canal N do pwm
    }

    for (uint8_t i = 0; i < ledsCount; i++) {
        // traduz RGB para GRB
        uint8_t r = colors[i].r;
        uint8_t g = colors[i].g;
        uint8_t b = colors[i].b;
        uint32_t color = (g << 16) | (r << 8) | b;  // GRB

        // escreve os dados do LED no formato WS2812
        for (uint8_t j = 0; j < 24; j++) {
            if (color & (1 << (23 - j))) {
                buffer[i * 5 * 8 * 3 + j * 5 + 0] = 0;  // bit 0
                buffer[i * 5 * 8 * 3 + j * 5 + 1] = 0;  // bit 1
                buffer[i * 5 * 8 * 3 + j * 5 + 2] = 0;  // bit 2
                buffer[i * 5 * 8 * 3 + j * 5 + 3] = 4;  // bit 3
                buffer[i * 5 * 8 * 3 + j * 5 + 4] = 4;  // bit 4
            } else {
                buffer[i * 5 * 8 * 3 + j * 5 + 0] = 0;  // bit 0
                buffer[i * 5 * 8 * 3 + j * 5 + 1] = 0;  // bit 1
                buffer[i * 5 * 8 * 3 + j * 5 + 2] = 4;  // bit 2
                buffer[i * 5 * 8 * 3 + j * 5 + 3] = 4;  // bit 3
                buffer[i * 5 * 8 * 3 + j * 5 + 4] = 4;  // bit 4
            }
        }
    }
    // envia os dados via DMA PWM
    HAL_TIMEx_PWMN_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)buffer, 5 * 8 * 3 * ledsCount + 200);
    // HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t*)buffer, 5 * 8 * 3 * ledsCount + 200);
}

#endif  // LED_WS2812_H
