#include "ESP32MotorControl.h"

static const char *TAG = "ESP32MotorControl";

void ESP32MotorControl::setSTBY(uint8_t _gpioSTBY)
{
  gpioSTBY = _gpioSTBY;
  gpio_set_direction((gpio_num_t)gpioSTBY, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)gpioSTBY, 1);
}

void ESP32MotorControl::PwmWrite(ledc_channel_t channel, int pwm){
    ledc_set_duty_and_update(LEDC_MODE,channel,pwm,0); // Atribui um novo duty para o PWM
}

void ESP32MotorControl::InitPWM(gpio_num_t pin, ledc_channel_t channel){
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode      = LEDC_MODE;
    ledc_timer.duty_resolution = LEDC_DUTY_RES;
    ledc_timer.timer_num       = LEDC_TIMER;
    ledc_timer.freq_hz         = LEDC_FREQUENCY; // Frequência de 5Khz
    ledc_timer.clk_cfg         = LEDC_AUTO_CLK; // Configuração da fonte de clock
    ledc_timer_config(&ledc_timer);

    // Prepara e aplica a configuração do canal do LEDC
    ledc_channel_config_t ledc_channel;
    ledc_channel.gpio_num       = pin;
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.channel        = channel;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.duty           = 0; 
    ledc_channel.hpoint         = 0; // Ponto de início do duty cycle
    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);
}

// Publics

void ESP32MotorControl::attachMotors(uint8_t _gpioAIN1, uint8_t _gpioAIN2,
                                     uint8_t _gpioPWMA, uint8_t _gpioBIN1,
                                     uint8_t _gpioBIN2, uint8_t _gpioPWMB)
{

  gpioAIN1 = _gpioAIN1;
  gpioAIN2 = _gpioAIN2;
  gpioPWMA = _gpioPWMA;
  gpioBIN1 = _gpioBIN1;
  gpioBIN2 = _gpioBIN2;
  gpioPWMB = _gpioPWMB;

  gpio_config_t io_conf;
  io_conf.intr_type = (gpio_int_type_t)GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ((1ULL << gpioAIN1) | (1ULL << gpioAIN2) | (1ULL << gpioBIN1) | (1ULL << gpioBIN2));
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);

  ESP_LOGD(TAG, "init PWM Motor 0");

  // Set MCPWM unit 0
  InitPWM((gpio_num_t)_gpioPWMA, PWM_A_PIN);

  this->mMotorAttached[0] = true;
  ESP_LOGD(TAG, "init PWM Motor 1");

  // Set MCPWM unit 1
  InitPWM((gpio_num_t)_gpioPWMB, PWM_B_PIN);

  this->mMotorAttached[1] = true;

  ESP_LOGD(TAG, "PWM initialized");
}

void ESP32MotorControl::motorSpeed(uint8_t motor, float speed)
{
  uint16_t DutyPwm = 0;
  switch (motor)
  {
  case 0:
    if(speed >= 0){
      if(mMotorState[0] != MotorState::MOTOR_FORWARD)
        this->motorForward(0);
    }
    else{
      if(mMotorState[0] != MotorState::MOTOR_REVERSE){
        this->motorReverse(0);
        speed = -speed;
      }
    }
    
    DutyPwm = (pow((float)2, (float)LEDC_DUTY_RES) - 1) * (speed/100.0);
    PwmWrite(PWM_A_PIN, DutyPwm);
    break;

  case 1:
    if(speed >= 0){
      if(mMotorState[1] != MotorState::MOTOR_FORWARD)
        this->motorForward(1);
    }
    else{
      if(mMotorState[1] != MotorState::MOTOR_REVERSE){
        this->motorReverse(1);
        speed = -speed;
      }
    }

    DutyPwm = (pow((float)2, (float)LEDC_DUTY_RES) - 1) * (speed/100.0);
    PwmWrite(PWM_B_PIN, DutyPwm);
    break;

  default:
    break;
  }

  ESP_LOGD(TAG, "Motor %u speed %f", motor, speed);
}

void ESP32MotorControl::motorForward(uint8_t motor)
{
  switch (motor)
  {
  case 0:
    gpio_set_level((gpio_num_t)gpioAIN1, 1);
    gpio_set_level((gpio_num_t)gpioAIN2, 0);
    break;

  case 1:
    gpio_set_level((gpio_num_t)gpioBIN1, 1);
    gpio_set_level((gpio_num_t)gpioBIN2, 0);
    break;

  default:
    break;
  }

  ESP_LOGD(TAG, "Motor %u set to forward", motor);
}

void ESP32MotorControl::motorReverse(uint8_t motor)
{
  switch (motor)
  {
  case 0:
    gpio_set_level((gpio_num_t)gpioAIN1, 0);
    gpio_set_level((gpio_num_t)gpioAIN2, 1);
    break;

  case 1:
    gpio_set_level((gpio_num_t)gpioBIN1, 0);
    gpio_set_level((gpio_num_t)gpioBIN2, 1);
    break;

  default:
    break;
  }

  ESP_LOGD(TAG, "Motor %u set to forward", motor);
}

void ESP32MotorControl::motorStop(uint8_t motor)
{
  switch (motor)
  {
  case 0:
    gpio_set_level((gpio_num_t)gpioAIN1, 0);
    gpio_set_level((gpio_num_t)gpioAIN2, 0);
    break;

  case 1:
    gpio_set_level((gpio_num_t)gpioBIN1, 0);
    gpio_set_level((gpio_num_t)gpioBIN2, 0);
    break;

  default:
    break;
  }

  ESP_LOGD(TAG, "Motor %u stop", motor);
}

void ESP32MotorControl::motorsStop()
{
  motorStop(0);
  motorStop(1);

  ESP_LOGD(TAG, "Motors stop");
}