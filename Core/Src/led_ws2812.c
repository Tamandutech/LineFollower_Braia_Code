#include "led_ws2812.h"

// send 24 bits of RGB color data to the led strip
void setLedsColor(pinhandler_t ledsPin, uint32_t* colors, uint8_t ledsCount) {
    write_pin(ledsPin, 0);
    delay_us(50);
    for (uint8_t i = 0; i < ledsCount; i++) {

        uint32_t color = colors[i];
        // converte RGB para GRB
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        color = (g << 16) | (r << 8) | b;  // GRB

        // envia do mais significativo para o menos significativo
        for (uint8_t j = 0; j < 24; j++) {
            if (color >> (23 - j)) {
                write_pin(ledsPin, 1);
                delay_ns(800);  // 0,8us high
                write_pin(ledsPin, 0);
                delay_ns(450);  // 0,4us low
            } else {
                write_pin(ledsPin, 1);
                delay_ns(400);  // 0,4us high
                write_pin(ledsPin, 0);
                delay_ns(850);  // 0,85us low
            }
        }
    }
}