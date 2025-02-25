/*
 * @Author: Caffreyfans
 * @Date: 2021-06-19 17:51:39
 * @LastEditTime: 2021-06-26 23:52:02
 * @Description:
 */
#include "web_handler.h"
#include "version.h"
#include "wifimanager.h"
// #include "ir.h"
#include "tools.h"

#define RET_BUFFER_SIZE 128
#define TMP_BUFFER_SIZE 64

static const char *TAG = "eventHandler";

#define CHECK_IS_NULL(value, ret) \
	if (value == NULL)            \
		return ret;

/*
 *获取系统信息
 **/
// todo 会触发 Guru Meditation Error: Core  0 panic'ed (Load access fault). Exception was unhandled.
static void get_info_handle(char **response)
{
	cJSON *root = cJSON_CreateObject();
	if (root == NULL)
		return;

	char ret_buffer[RET_BUFFER_SIZE];
	char tmp_buffer[TMP_BUFFER_SIZE];
	extern int debug;

	int64_t tick = xTaskGetTickCount();
	snprintf(ret_buffer, RET_BUFFER_SIZE, "%ldS", (unsigned long int) pdTICKS_TO_MS(tick) / 1000);
	cJSON_AddStringToObject(root, "boot", ret_buffer);

	itoa(heap_caps_get_total_size(MALLOC_CAP_32BIT) / 1024, tmp_buffer, 10);
	snprintf(ret_buffer, RET_BUFFER_SIZE, "%sKB", tmp_buffer);
	cJSON_AddStringToObject(root, "total_heap", ret_buffer);

	itoa(heap_caps_get_free_size(MALLOC_CAP_32BIT) / 1024, tmp_buffer, 10);
	snprintf(ret_buffer, RET_BUFFER_SIZE, "%sKB", tmp_buffer);
	cJSON_AddStringToObject(root, "free_heap", ret_buffer);

	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);
	cJSON_AddStringToObject(root, "wifi_ssid", (char *)ap.ssid);

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(g_station_netif, &ip_info);
	snprintf(ret_buffer, RET_BUFFER_SIZE, IPSTR, IP2STR(&ip_info.ip));
	cJSON_AddStringToObject(root, "wifi_ip", ret_buffer);

	uint8_t mac[6];
	esp_read_mac(mac, ESP_MAC_WIFI_STA);
	snprintf(ret_buffer, RET_BUFFER_SIZE, MACSTR, MAC2STR(mac));
	cJSON_AddStringToObject(root, "wifi_mac", ret_buffer);

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%d", ap.rssi);
	cJSON_AddStringToObject(root, "wifi_rssi", ret_buffer);

	cJSON_AddStringToObject(root, "build_time", __DATE__);

	// cJSON_AddStringToObject(root, "sketch_size", );

	// cJSON_AddStringToObject(root, "free_sketch_size", );

	cJSON_AddStringToObject(root, "core_version", CORE_VERSION);


	cJSON_AddStringToObject(root, "sdk_version", esp_get_idf_version());

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%ldMHz", (unsigned long int) ets_get_cpu_frequency());
	cJSON_AddStringToObject(root, "cpu_freq", ret_buffer);

	uint32_t size_flash_chip;
	esp_flash_get_size(NULL, &size_flash_chip);

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%ldMB",
			 (unsigned long int) size_flash_chip / 1024 / 1024);
	cJSON_AddStringToObject(root, "flash_size", ret_buffer);

	// cJSON_AddStringToObject(root, "flash_speed", );
	size_t total_bytes, used_bytes;
	esp_spiffs_info("spiffs", &total_bytes, &used_bytes);

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%zuKB", total_bytes / 1024);
	cJSON_AddStringToObject(root, "fs_total", ret_buffer);

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%zuKB", used_bytes / 1024);
	cJSON_AddStringToObject(root, "fs_used", ret_buffer);

	extern temperature_sensor_handle_t temp_handle;
	// 启用温度传感器
	ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
	// 获取传输的传感器数据
	float tsens_out;
	ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
	snprintf(ret_buffer, RET_BUFFER_SIZE, "%.1f", tsens_out);
	cJSON_AddStringToObject(root, "core_t", ret_buffer);
	// 温度传感器使用完毕后，禁用温度传感器，节约功耗
	ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));

	snprintf(ret_buffer, RET_BUFFER_SIZE, "%d", debug);
	cJSON_AddStringToObject(root, "is_debug", ret_buffer);


	static const char *reset_reason_str[] = {"can not be determined",
											 "power-on event",
											 "external pin",
											 "esp_restart",
											 "exception/panic",
											 "interrupt watchdog",
											 "task watchdog",
											 "other watchdogs",
											 "exiting deep sleep mode",
											 "browout reset",
											 "SDIO",
											 "USB peripheral",
											 "JTAG",
											 "efuse error",
											 "power glitch detected",
											 "CPU lock up"};
/*
	ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
	ESP_RST_POWERON,    //!< Reset due to power-on event
	ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
	ESP_RST_SW,         //!< Software reset via esp_restart
	ESP_RST_PANIC,      //!< Software reset due to exception/panic
	ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
	ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
	ESP_RST_WDT,        //!< Reset due to other watchdogs
	ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
	ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
	ESP_RST_SDIO,       //!< Reset over SDIO
	ESP_RST_USB,        //!< Reset by USB peripheral
	ESP_RST_JTAG,       //!< Reset by JTAG
	ESP_RST_EFUSE,      //!< Reset due to efuse error
	ESP_RST_PWR_GLITCH, //!< Reset due to power glitch detected
	ESP_RST_CPU_LOCKUP, //!< Reset due to CPU lock up (double exception)
*/

	cJSON_AddStringToObject(root, "reset_reason",
							reset_reason_str[esp_reset_reason()]);

	*response = cJSON_Print(root);
	cJSON_Delete(root);
}

/*
 *打印可用堆内存*/
void print_free_heap()
{
	char ret_buffer[RET_BUFFER_SIZE];
	char tmp_buffer[TMP_BUFFER_SIZE];
	itoa(heap_caps_get_free_size(MALLOC_CAP_32BIT) / 1024, tmp_buffer, 10);
	snprintf(ret_buffer, RET_BUFFER_SIZE, "%sKB", tmp_buffer);
	ESP_LOGI(TAG, "free_heap %s", ret_buffer);
}

void power_info_print(char *value) {
	// 将读取到的json字符串转换成json变量指针
	cJSON *root = cJSON_Parse(value);
	if (!root)
	{
		ESP_LOGW(TAG, "Error before: [%s]", cJSON_GetErrorPtr());
		return;
	}

	cJSON *ap_item = NULL;
	cJSON *total_e__item = NULL;
	char *ap = NULL;
	char *total_e = NULL;

	ap_item = cJSON_GetObjectItem(root, "ap");
	if (ap_item != NULL)
	{
		// 判断是不是字符串类型
		if (ap_item->type == cJSON_String)
		{
			ap = ap_item->valuestring; // 此赋值是浅拷贝，不需要现在释放内存
			printf("ap = %s\n", ap);
		}
	}

	total_e__item = cJSON_GetObjectItem(root, "total_e");
	if (total_e__item != NULL)
	{
		// 判断是不是字符串类型
		if (total_e__item->type == cJSON_String)
		{
			total_e = total_e__item->valuestring; // 此赋值是浅拷贝，不需要现在释放内存
			printf("total_e = %s\n", total_e);
		}
	}
	char *buffer[40];
	sprintf("power: %s\r\ntotal:%s", ap, total_e);
	lcd_print(buffer);
	
	}

/*
 *事件处理接口*/
char *index_handler(int key, char *value)
{
	char *response_t = NULL;
	switch (key)
	{
	case 0: //
		power_info_print(value);
		break;
	default:
		get_info_handle(&response_t);
		break;
	}
	extern int debug;
	if (debug)
		ESP_LOGI(TAG, "key: %d, response: %s", key, response_t);
	return response_t;
}