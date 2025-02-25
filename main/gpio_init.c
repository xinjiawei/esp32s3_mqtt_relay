#include "gpio_init.h"

void gpio_init(void) {

	// 配置button GPIO
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_ANYEDGE; // 双边沿触发中断
	io_conf.pin_bit_mask = (1ULL << BUTTON_GPIO_PIN);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1; // 启用上拉电阻
	gpio_config(&io_conf);

	// 配置buzzer gpio
	io_conf.intr_type = GPIO_INTR_DISABLE;
	// set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	// bit mask of the pins that you want to set,e.g.GPIO5
	io_conf.pin_bit_mask = (1ULL << BUZZER_GPIO_PIN);
	// disable pull-down mode
	io_conf.pull_down_en = 0;
	// disable pull-up mode
	io_conf.pull_up_en = 0;
	// configure GPIO with the given settings
	gpio_config(&io_conf);
}