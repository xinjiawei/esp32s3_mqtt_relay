#pragma once
#define BUTTON_GPIO_PIN 4 // 按钮连接的 GPIO 引脚

#define BUZZER_GPIO_PIN 5 // 蜂鸣器连接的 GPIO 引脚


#include "driver/gpio.h"

void gpio_init(void);