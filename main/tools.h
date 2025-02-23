#pragma once
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

#include "led.h"
#include "lcd.h"

#include "zh_syslog.h"

#ifdef ESP_LOG_LEVEL_LOCAL
#undef ESP_LOG_LEVEL_LOCAL
#endif

#ifdef CONFIG_LOG_MASTER_LEVEL
#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...) \
	do                                                                           \
	{                                                                            \
		if ((esp_log_get_level_master() >= level) && (LOG_LOCAL_LEVEL >= level)) \
			ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__);                    \
	} while (0)
#else
#define ESP_LOG_LEVEL_LOCAL(level, tag, format, ...)                \
	do                                                              \
	{                                                               \
		if (LOG_LOCAL_LEVEL >= level){ zh_syslog_send(ZH_USER, ZH_INFO, "esp32s3", "Message"); ESP_LOG_LEVEL(level, tag, format, ##__VA_ARGS__);} \
	} while (0)
#endif // CONFIG_LOG_MASTER_LEVEL

void filesys_init();
void create_ota_tag();
void remove_ota_tag();
int is_exist_ota_tag();

char *get_len_str(char *original, int len);
char * get_chip_id();

void print_sys_info();
unsigned int crc_cal(const char *pBuff, int len);

float float_from_8hex(int arr[]);
void print_ddsu666_params(uint8_t bytes[], volatile float *voltage, volatile float *current,
						  volatile float *a_power, volatile float *r_power, volatile float *ap_power,
						  volatile float *power_factor, volatile float *power_frequency);
void print_ddsu666_total_energy(uint8_t bytes[], volatile float *total_energy);
void debug_switch();
void led_loop(int times);