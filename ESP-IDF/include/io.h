
// Sensores laterais
#define SL1 ADC1_CHANNEL_3 // GPIO39
#define SL2 ADC1_CHANNEL_0 // GPIO36
#define SL3 ADC1_CHANNEL_4 // GPIO32
#define SL4 ADC1_CHANNEL_5 // GPIO33

// TB6612FNG Driver
#define DRIVER_PWMA 21
#define DRIVER_AIN1 26
#define DRIVER_AIN2 25
#define DRIVER_PWMB 14
#define DRIVER_BIN2 27
#define DRIVER_BIN1 13
#define DRIVER_STBY 15

// Encoders Motores
#define ENC_MOT_ESQ_A 34
#define ENC_MOT_ESQ_B 35
#define ENC_MOT_DIR_A 04
#define ENC_MOT_DIR_B 16

// SPI do ADC (Utiliza VSPI do ESP32)
#define ADC_CLK 18
#define ADC_DOUT 19
#define ADC_DIN 23
#define ADC_CS 22