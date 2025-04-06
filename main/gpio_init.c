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

	// 配置继电器 gpio
	io_conf.intr_type = GPIO_INTR_DISABLE;
	// set as output mode
	io_conf.mode = GPIO_MODE_OUTPUT;
	// bit mask of the pins that you want to set,e.g.GPIO5
	io_conf.pin_bit_mask = ((1ULL << SWITCH_GPIO_PIN1) | (1ULL << SWITCH_GPIO_PIN2));
	// disable pull-down mode
	io_conf.pull_down_en = 0;
	// disable pull-up mode
	io_conf.pull_up_en = 0;
	// configure GPIO with the given settings
	gpio_config(&io_conf);

	// 8-segment display 初始化 LATCH 引脚
	gpio_config_t spi_io_conf = {
		.pin_bit_mask = (1ULL << PIN_NUM_LATCH),
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = 0,
		.pull_down_en = 0,
		.intr_type = GPIO_INTR_DISABLE};
	gpio_config(&spi_io_conf);
	gpio_set_level(PIN_NUM_LATCH, 1);

	// 8-segment display SPI 配置
	spi_bus_config_t buscfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = -1,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 2};
	spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 1 * 1000 * 1000, // 1MHz
		.mode = 0,
		.spics_io_num = -1, // 我们手动控制 LATCH
		.queue_size = 1};

	extern spi_device_handle_t spi;
	spi_bus_add_device(SPI2_HOST, &devcfg, &spi);

}