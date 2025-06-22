/********************************************************************************
 * @file  WS2812Driver.h
 * @author samuelc254
 * @brief  Driver para LEDs endereçáveis WS2812B usando TIM PWM e DMA.
 * @note  Este driver foi desenvolvido para a Celeris Core S1
 * @note  e depende da configuração do timer TIM1 no CubeMX.
 * @note  Certifique-se de usar o .ioc correto para a configuração do projeto.
 * Esta implementação usa um PWM de alta frequência (6.4 MHz) para
 * gerar a temporização precisa de 800 kHz para o barramento de dados.
 *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "stm32g4xx_hal.h"

// Definição da estrutura de cor RGB
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

/* --- Configuração do Driver --- */
// Defina o número máximo de LEDs que sua fita terá.
// O buffer de memória será alocado estaticamente para este tamanho.
#define MAX_LEDS 4

// O timer está configurado para 6.4MHz, 8x a frequência de dados de 800kHz do WS2812
#define PWM_CYCLES_PER_BIT 8

#define LED_BITS 24  // 8 bits para G, 8 para R, 8 para B

// O pulso de reset precisa de pelo menos 50us.
// Nosso ciclo de PWM é de 156.25ns (1/6.4MHz).
// 50,000ns / 156.25ns = 320 ciclos. Usaremos um pouco mais por segurança.
#define RESET_CYCLES 330

// Valores do registrador CCR para gerar os níveis lógicos no canal N (invertido)
#define PWM_HIGH 0  // Para o canal N ficar ALTO, o canal principal deve ter 0% de duty cycle
#define PWM_LOW 5   // Para o canal N ficar BAIXO, o canal principal deve ter 100% de duty cycle (ARR=4, CCR=5)

/* --- Buffer Estático para DMA --- */
// Buffer principal. Declarado como 'static' para evitar estouro de stack.
// O tamanho é calculado com base no máximo de LEDs e no pulso de reset.
// Usamos uint32_t para corresponder ao tamanho do registrador do timer (CCR).
static uint32_t pwm_buffer[RESET_CYCLES + MAX_LEDS * LED_BITS * PWM_CYCLES_PER_BIT + RESET_CYCLES];

// Handle do Timer, declarado como 'extern' pois é definido pelo CubeMX em main.c
extern TIM_HandleTypeDef htim1;

/**
 * @brief Converte e envia um array de cores para a fita de LEDs.
 * @param colors Ponteiro para um array de structs rgb_color_t.
 * @param ledsCount O número de LEDs a serem atualizados.
 */

void setLedsColor(rgb_color_t *colors, uint16_t ledsCount) {
    // 1. Prevenir overflow do buffer se a contagem de LEDs for muito alta
    if (ledsCount > MAX_LEDS) {
        ledsCount = MAX_LEDS;
    }

    // Ponteiro para o início do nosso buffer DMA
    uint32_t *buffer_ptr = pwm_buffer;

    // Adiciona pulso de RESET no início
    for (int i = 0; i < RESET_CYCLES; i++) {
        *buffer_ptr++ = PWM_LOW;
    }

    // 2. Loop através de cada LED
    for (uint16_t i = 0; i < ledsCount; i++) {
        // Garante que os valores sejam pares pois por algum motivo valores ímpares causam problemas
        uint8_t r = colors[i].r % 2 ? colors[i].r - 1 : colors[i].r;
        uint8_t g = colors[i].g % 2 ? colors[i].g - 1 : colors[i].g;
        uint8_t b = colors[i].b % 2 ? colors[i].b - 1 : colors[i].b;

        // Formato GRB é o padrão para a maioria dos chips WS2812
        uint32_t color = (g << 16) | (r << 8) | b;

        // 3. Loop através dos 24 bits de cor de cada LED (G7..G0, R7..R0, B7..B0)
        for (int j = 23; j >= 0; j--) {
            if ((color >> j) & 1) {
                // Padrão de bit '1': ALTO por ~780ns, BAIXO por ~470ns
                // Para o canal N, isso significa 5 ciclos BAIXO e 3 ALTOS no canal principal.
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;

            } else {
                // Padrão de bit '0': ALTO por ~470ns, BAIXO por ~780ns
                // Para o canal N, isso significa 3 ciclos BAIXO e 5 ALTOS no canal principal.
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
            }
        }
    }

    // 4. Preencher o fim do buffer com o pulso de RESET
    for (int i = 0; i < RESET_CYCLES; i++) {
        *buffer_ptr++ = PWM_LOW;
    }

    // 5. Enviar os dados via DMA
    // O tamanho total dos dados a serem enviados é o reset inicial, os LEDs e o reset final.
    uint32_t data_len = ledsCount * LED_BITS * PWM_CYCLES_PER_BIT;
    uint32_t total_buffer_size = RESET_CYCLES + data_len + RESET_CYCLES;

    // Inicia a transferência DMA
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, pwm_buffer, total_buffer_size);
}

/**
 * @brief Exemplo de como usar a função em seu main.c
 */
/*
    rgb_color_t led;
    for (;;) {
        led = (rgb_color_t){0, 0, 128};  // Inicializa o LED com azul
        setLedsColor(&led, 1);
        delay_ms(500);
        led = (rgb_color_t){0, 128, 0};  // Muda o LED para verde
        setLedsColor(&led, 1);
        delay_ms(500);
        led = (rgb_color_t){128, 0, 0};  // Muda o LED para vermelho
        setLedsColor(&led, 1);
        delay_ms(500);
    }
*/

#ifdef __cplusplus
}
#endif