/* SPI Master Half Duplex EEPROM example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "8sedisplay.h"

static const char logTAG[] = "8-segment Display";

spi_device_handle_t spi;

// 数码管段码（共阳）
static const uint8_t digit_table[] = {
	0b11000000, // 0
	0b11111001, // 1
	0b10100100, // 2
	0b10110000, // 3
	0b10011001, // 4
	0b10010010, // 5
	0b10000010, // 6
	0b11111000, // 7
	0b10000000, // 8
	0b10010000, // 9

	0b01111111, // dot
};

void latch()
{
	gpio_set_level(PIN_NUM_LATCH, 0);
	esp_rom_delay_us(1);
	gpio_set_level(PIN_NUM_LATCH, 1);
	esp_rom_delay_us(1);
}

void send_2_digits(uint8_t d1, uint8_t d2)
{
	int dot_tagger = d2 % 2 == 0 ? 0b10000000 : 0b00000000;							// 0b10000000 表示不点亮小数点
	uint8_t data[2] = {digit_table[d2] ^ dot_tagger, digit_table[d1] ^ dot_tagger}; // 高位在前
	spi_transaction_t t = {
		.length = 16, // 2 字节
		.tx_buffer = data};
	spi_device_transmit(spi, &t);
	latch();
	ESP_LOGI(logTAG, "send_2_digits: %d %d", d1, d2);
}

