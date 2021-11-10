// Sensores laterais
#define SL1 ADC1_CHANNEL_3// GPIO39
#define SL2 ADC1_CHANNEL_0 // GPIO36
#define SL3 17 // GPIO17
#define SL4 05 // GPIO05

// TB6612FNG Driver
#define DRIVER_PWMA 14
#define DRIVER_AIN1 26
#define DRIVER_AIN2 27
#define DRIVER_PWMB 32
#define DRIVER_BIN2 33
#define DRIVER_BIN1 25
#define DRIVER_STBY 21

// Encoders Motores
#define ENC_MOT_ESQ_A 04
#define ENC_MOT_ESQ_B 16
#define ENC_MOT_DIR_A 35
#define ENC_MOT_DIR_B 34

// SPI do ADC (Utiliza VSPI do ESP32)
#define ADC_CLK 18
#define ADC_DOUT 19
#define ADC_DIN 23
#define ADC_CS 22