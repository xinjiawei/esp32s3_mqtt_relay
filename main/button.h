#pragma once
#include "tools.h"
#include "gpio_init.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stdio.h>

void button_init(void);