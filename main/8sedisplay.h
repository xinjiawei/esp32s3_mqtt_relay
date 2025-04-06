#pragma once
#include "gpio_init.h"

#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

void send_2_digits(uint8_t d1, uint8_t d2);