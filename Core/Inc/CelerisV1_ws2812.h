/**

******************************************************************************

* @file  led_driver.c

* @author Samuel Oliveira

* @brief  Driver para LEDs endereçáveis WS2812B usando TIM PWM e DMA.

* Esta implementação usa um PWM de alta frequência (4 MHz) para

* gerar a temporização precisa de 800 kHz para o barramento de dados.

******************************************************************************

*/

#include <stdint.h>
#include <string.h>  // Para usar memset

// Inclua o header principal do seu projeto se necessário, ou diretamente o da HAL

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

// O timer está configurado para 4MHz, 5x a frequência de dados de 800kHz do WS2812

#define PWM_CYCLES_PER_BIT 5

#define LED_BITS 24  // 8 bits para G, 8 para R, 8 para B

// O pulso de reset precisa de pelo menos 50us.

// Nosso ciclo de PWM é de 250ns (1/4MHz).

// 50,000ns / 250ns = 200 ciclos. Usaremos um pouco mais por segurança.

#define RESET_CYCLES 250

// Valores do registrador CCR para gerar os níveis lógicos no canal N (invertido)

#define PWM_HIGH 0  // Para o canal N ficar ALTO, o canal principal deve ter 0% de duty cycle

#define PWM_LOW 4  // Para o canal N ficar BAIXO, o canal principal deve ter 100% de duty cycle (ARR=3, então CCR=4)

/* --- Buffer Estático para DMA --- */

// Buffer principal. Declarado como 'static' para evitar estouro de stack.

// O tamanho é calculado com base no máximo de LEDs e no pulso de reset.

// Usamos uint32_t para corresponder ao tamanho do registrador do timer (CCR).

static uint32_t pwm_buffer[MAX_LEDS * LED_BITS * PWM_CYCLES_PER_BIT + RESET_CYCLES];

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

    // 2. Loop através de cada LED

    for (uint16_t i = 0; i < ledsCount; i++) {
        // Formato GRB é o padrão para a maioria dos chips WS2812

        uint32_t color = (colors[i].g << 16) | (colors[i].r << 8) | colors[i].b;

        // 3. Loop através dos 24 bits de cor de cada LED (G7..G0, R7..R0, B7..B0)

        for (int j = 23; j >= 0; j--) {
            if ((color >> j) & 1) {
                // Padrão de bit '1': ALTO por ~800ns, BAIXO por ~450ns

                // Para o canal N, isso significa 3 ciclos BAIXO e 2 ALTOS no canal principal.

                *buffer_ptr++ = PWM_HIGH;

                *buffer_ptr++ = PWM_HIGH;

                *buffer_ptr++ = PWM_HIGH;

                *buffer_ptr++ = PWM_LOW;

                *buffer_ptr++ = PWM_LOW;

            } else {
                // Padrão de bit '0': ALTO por ~400ns, BAIXO por ~850ns

                // Para o canal N, isso significa 2 ciclos BAIXO e 3 ALTOS no canal principal.

                *buffer_ptr++ = PWM_HIGH;

                *buffer_ptr++ = PWM_LOW;

                *buffer_ptr++ = PWM_LOW;

                *buffer_ptr++ = PWM_LOW;

                *buffer_ptr++ = PWM_LOW;
            }
        }
    }

    // 4. Preencher o resto do buffer com o pulso de RESET

    // O ponteiro 'buffer_ptr' agora está no final dos dados dos LEDs.

    // Usamos 'memset' para preencher com o valor 'LOW' eficientemente.

    // O número de ciclos de reset é o que sobrou no buffer após os dados dos leds.

    // Garantimos um mínimo de RESET_CYCLES.

    uint32_t data_len = ledsCount * LED_BITS * PWM_CYCLES_PER_BIT;

    memset(&pwm_buffer[data_len], PWM_LOW, RESET_CYCLES * sizeof(uint32_t));

    // 5. Enviar os dados via DMA

    // O tamanho total dos dados a serem enviados é o dos LEDs mais o reset.

    uint32_t total_buffer_size = data_len + RESET_CYCLES;

    // Primeiro, para qualquer transferência em andamento. Essencial para evitar conflitos.

    HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);

    // Inicia a transferência DMA

    if (HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, pwm_buffer, total_buffer_size) != HAL_OK) {
        // Tratar erro
    }

    // Habilita a saída no pino do canal complementar (N)

    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        // Tratar erro
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