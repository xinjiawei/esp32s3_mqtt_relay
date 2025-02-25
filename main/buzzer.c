#include "buzzer.h"

void buzzer(void) {
	gpio_set_level(BUZZER_GPIO_PIN, 1);
	vTaskDelay(30 / portTICK_PERIOD_MS);
	gpio_set_level(BUZZER_GPIO_PIN, 0);
}