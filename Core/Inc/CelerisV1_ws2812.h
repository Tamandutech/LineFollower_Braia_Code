/**
 ******************************************************************************
 * @file    led_driver.c
 * @author  Samuel Oliveira
 * @brief   Driver para LEDs endereçáveis WS2812B usando TIM PWM e DMA.
 * @version 2.1 - Implementação de alta precisão com 6.4MHz
 ******************************************************************************
 *
 * NOTA DE CONFIGURAÇÃO (v2.1 - 6.4MHz):
 * Esta versão usa um PWM de alta frequência (6.4 MHz) para obter a máxima
 * precisão na temporização dos bits. Para que este código funcione,
 * as configurações do TIM1 no CubeMX DEVEM ser atualizadas para:
 *
 * - ative o modo de PWM no canal TIM1_CH1N
 *
 * - Prescaler (PSC): 5-1 (4)
 * - Counter Period (ARR): 5-1 (4)
 *
 * - Ative o DMA no canal TIM1_CH1
 * - configure o DMA para:
 *    - Modo: Normal
 *    - Direção: Memória para Periférico
 *
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <string.h>  // Para usar memset

// Inclua o header principal do seu projeto se necessário, ou diretamente o da HAL
#include "stm32g4xx_hal.h"

// Definição da estrutura de cor RGB para facilitar a manipulação.
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

/* --- Configuração do Driver (Baseado em 6.4MHz PWM) --- */

// Defina o número máximo de LEDs que sua fita terá.
// O buffer de memória será alocado estaticamente para este tamanho.
#define MAX_LEDS 4

// O protocolo WS2812B opera a 800kHz (período de 1250ns).
// Usamos um PWM 8x mais rápido (6.4MHz) para ter 8 "fatias de tempo" para construir cada bit.
// Cada "fatia" ou ciclo de PWM dura 156.25ns. 8 * 156.25ns = 1250ns.
#define PWM_CYCLES_PER_BIT 8
#define LED_BITS 24  // Cada cor (R, G, B) usa 8 bits.

// O pulso de reset para "travar" as cores nos LEDs precisa de pelo menos 50us.
// Nosso ciclo de PWM dura 156.25ns.
// 50,000ns / 156.25ns = 320 ciclos. Este é o número de pulsos de baixa que enviaremos no final.
#define RESET_CYCLES 320

// Com ARR=4, o contador vai de 0 a 4 (5 passos). O valor para 100% de duty cycle é 5.
#define PWM_HIGH 0  // Para o canal N (invertido) ficar ALTO, o canal principal deve ter 0% de duty cycle.
#define PWM_LOW 5   // Para o canal N ficar BAIXO, o canal principal deve ter 100% de duty cycle.

/* --- Buffer Estático para DMA --- */

// O buffer que conterá a sequência de valores de duty cycle para o DMA.
// Declarado como 'static' para ser alocado na memória RAM estática e evitar estouro da stack.
static uint32_t pwm_buffer[MAX_LEDS * LED_BITS * PWM_CYCLES_PER_BIT + RESET_CYCLES];

// O "handle" do Timer 1, que é configurado pelo CubeMX e definido no main.c.
// Usamos 'extern' para dizer ao compilador que esta variável existe em outro arquivo.
extern TIM_HandleTypeDef htim1;

/**
 * @brief Converte um array de cores para o formato de pulso do WS2812B e inicia a transmissão via DMA.
 * @param colors Ponteiro para um array de structs rgb_color_t contendo as cores a serem exibidas.
 * @param ledsCount O número de LEDs a serem atualizados (não deve exceder MAX_LEDS).
 */
void setLedsColor(rgb_color_t* colors, uint16_t ledsCount) {
    // 1. GARANTIA DE SEGURANÇA: Limita a contagem de LEDs ao tamanho máximo do nosso buffer
    // para evitar escrita em memória indevida (buffer overflow).
    if (ledsCount > MAX_LEDS) {
        ledsCount = MAX_LEDS;
    }

    // Ponteiro auxiliar para preencher o buffer de forma eficiente.
    uint32_t* buffer_ptr = pwm_buffer;

    // 2. CONVERSÃO DE COR PARA PULSOS: Itera sobre cada LED.
    for (uint16_t i = 0; i < ledsCount; i++) {
        // O chip WS2812B espera os dados na ordem Verde, Vermelho, Azul (GRB).
        // Combinamos os bytes de cor em uma única variável de 32 bits para facilitar a manipulação.
        uint32_t color = (colors[i].g << 16) | (colors[i].r << 8) | colors[i].b;

        // Itera sobre cada um dos 24 bits de cor, do mais significativo (G7) ao menos significativo (B0).
        for (int j = 23; j >= 0; j--) {
            if ((color >> j) & 1) {
                // Se o bit for '1', geramos um pulso ALTO de ~781ns.
                // Isso corresponde a 5 ciclos de PWM em nível ALTO e 3 em BAIXO.
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
            } else {
                // Se o bit for '0', geramos um pulso ALTO de ~312ns.
                // Isso corresponde a 2 ciclos de PWM em nível ALTO e 6 em BAIXO.
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_HIGH;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
                *buffer_ptr++ = PWM_LOW;
            }
        }
    }

    // 3. ADIÇÃO DO PULSO DE RESET: Após os dados de todos os LEDs, o protocolo exige
    // um pulso baixo de >50us para que os LEDs "travem" a cor.
    // O 'memset' preenche eficientemente o final do buffer com o valor de PWM baixo.
    uint32_t data_len = ledsCount * LED_BITS * PWM_CYCLES_PER_BIT;
    memset(&pwm_buffer[data_len], PWM_LOW, RESET_CYCLES * sizeof(uint32_t));

    // 4. INÍCIO DA TRANSMISSÃO DMA:
    // Calcula o tamanho total do pacote a ser enviado (dados dos LEDs + pulso de reset).
    uint32_t total_buffer_size = data_len + RESET_CYCLES;

    // Para qualquer transferência anterior para garantir que o canal DMA está livre.
    // Também desliga as saídas PWM para evitar pulsos espúrios.
    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

    // Inicia a transferência do nosso buffer para o registrador de duty cycle do timer.
    // O DMA agora opera em segundo plano, sem usar a CPU.
    if (HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, pwm_buffer, total_buffer_size) != HAL_OK) {
        // Tratar erro, por exemplo, acendendo um LED de erro.
    }

    // Habilita a saída física no pino do canal complementar (N) para que o sinal seja transmitido.
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        // Tratar erro.
    }
}

/**
 * @brief Exemplo de como usar a função em seu main.c
 * Lembre-se que delay_ms() deve ser substituído por uma função de delay
 * real, como HAL_Delay().
 */
/*
void main_example() {

    // Em algum lugar no seu loop principal
    rgb_color_t led_color;
    while(1) {
        // LED: Azul
        led_color.r = 0;
        led_color.g = 0;
        led_color.b = 128;
        setLedsColor(&led_color, 1);
        HAL_Delay(500);

        // LED: Verde
        led_color.r = 0;
        led_color.g = 128;
        led_color.b = 0;
        setLedsColor(&led_color, 1);
        HAL_Delay(500);

        // LED: Vermelho
        led_color.r = 128;
        led_color.g = 0;
        led_color.b = 0;
        setLedsColor(&led_color, 1);
        HAL_Delay(500);
    }
}
*/
