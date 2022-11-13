#include "ESP32MotorControl.h"

static const char *TAG = "ESP32MotorControl";

void ESP32MotorControl::setSTBY(uint8_t _gpioSTBY)
{
  gpioSTBY = _gpioSTBY;
#ifndef ESP32_QEMU
  gpio_set_direction((gpio_num_t)gpioSTBY, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)gpioSTBY, 1);
#endif
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
  io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ((1ULL << gpioAIN1) | (1ULL << gpioAIN2) | (1ULL << gpioBIN1) | (1ULL << gpioBIN2));
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
#ifndef ESP32_QEMU
  gpio_config(&io_conf);
#endif

  ESP_LOGD(TAG, "init MCPWM Motor 0");

  // Set MCPWM unit 0
#ifndef ESP32_QEMU
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, gpioPWMA);
  // gpio_set_direction((gpio_num_t)gpioAIN1, GPIO_MODE_OUTPUT);
  // gpio_set_direction((gpio_num_t)gpioAIN2, GPIO_MODE_OUTPUT);
#endif

  this->mMotorAttached[0] = true;
  ESP_LOGD(TAG, "init MCPWM Motor 1");

  // Set MCPWM unit 1
#ifndef ESP32_QEMU
  mcpwm_gpio_init(MCPWM_UNIT_1, MCPWM1A, gpioPWMB);
  // gpio_set_direction((gpio_num_t)gpioBIN1, GPIO_MODE_OUTPUT);
  // gpio_set_direction((gpio_num_t)gpioBIN2, GPIO_MODE_OUTPUT);
#endif

  this->mMotorAttached[1] = true;
  ESP_LOGD(TAG, "Configuring Initial Parameters of MCPWM...");

  mcpwm_config_t pwm_config;
  pwm_config.frequency = PWM_FREQ; // frequency,
  pwm_config.cmpr_a = 0;           // duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;           // duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
#ifndef ESP32_QEMU
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0,
             &pwm_config); // Configure PWM0A & PWM0B with above settings

  mcpwm_init(MCPWM_UNIT_1, MCPWM_TIMER_1,
             &pwm_config); // Configure PWM1A & PWM1B with above settings
#endif

  ESP_LOGD(TAG, "MCPWM initialized");
}

void ESP32MotorControl::motorSpeed(uint8_t motor, float speed)
{
  switch (motor)
  {
  case 0:
#ifndef ESP32_QEMU
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

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, speed);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A,
                        MCPWM_DUTY_MODE_0); // call this each time, if operator
                                            // was previously in low/high state
#endif
    break;

  case 1:
#ifndef ESP32_QEMU
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

    mcpwm_set_duty(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, speed);
    mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A,
                        MCPWM_DUTY_MODE_0); // call this each time, if operator
                                            // was previously in low/high state
#endif
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
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioAIN1, 1);
    gpio_set_level((gpio_num_t)gpioAIN2, 0);
#endif
    break;

  case 1:
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioBIN1, 1);
    gpio_set_level((gpio_num_t)gpioBIN2, 0);
#endif
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
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioAIN1, 0);
    gpio_set_level((gpio_num_t)gpioAIN2, 1);
#endif
    break;

  case 1:
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioBIN1, 0);
    gpio_set_level((gpio_num_t)gpioBIN2, 1);
#endif
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
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioAIN1, 0);
    gpio_set_level((gpio_num_t)gpioAIN2, 0);
#endif
    break;

  case 1:
#ifndef ESP32_QEMU
    gpio_set_level((gpio_num_t)gpioBIN1, 0);
    gpio_set_level((gpio_num_t)gpioBIN2, 0);
#endif
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