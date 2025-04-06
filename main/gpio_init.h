#pragma once
#define BUTTON_GPIO_PIN 4 // 按钮连接的 GPIO 引脚

#define BUZZER_GPIO_PIN 5 // 蜂鸣器连接的 GPIO 引脚

#define SWITCH_GPIO_PIN1 6 // 蜂鸣器连接的 GPIO 引脚
#define SWITCH_GPIO_PIN2 7 // 蜂鸣器连接的 GPIO 引脚

// ir引脚
#define EXAMPLE_IR_TX_GPIO_NUM 5
#define EXAMPLE_IR_RX_GPIO_NUM 42

// lcd 屏幕
#define EXAMPLE_PIN_NUM_SDA 2
#define EXAMPLE_PIN_NUM_SCL 1
#define EXAMPLE_PIN_NUM_RST -1

// SPI 引脚定义（使用默认 VSPI）
#define PIN_NUM_MOSI 44
#define PIN_NUM_CLK 43
#define PIN_NUM_LATCH 41


#include "driver/gpio.h"
#include "driver/spi_master.h"

void gpio_init(void);