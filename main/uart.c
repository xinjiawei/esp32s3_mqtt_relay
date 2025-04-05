#include "uart.h"
#include "tools.h"
#include "watchdog.h"
/**
 * This is a example which echos any data it receives on UART back to the sender using RS485 interface in half duplex mode.
 */
static const char *TAG = "RS485_uart";
// Note: Some pins on target chip cannot be assigned for UART communication.
// Please refer to documentation for selected board and target to configure pins using Kconfig.
#define ECHO_TEST_TXD (5)
#define ECHO_TEST_RXD (6)

// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (127)

// Read packet timeout
#define PACKET_READ_TICS (100 / portTICK_PERIOD_MS)
#define ECHO_TASK_STACK_SIZE (2048)
#define ECHO_TASK_PRIO (10)
#define ECHO_UART_PORT (1)

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

const static int uart_num = ECHO_UART_PORT;
typedef unsigned char byte;

volatile float voltage = 0.0;
volatile float current = 0.0;
volatile float a_power = 0.0;
volatile float r_power = 0.0;
volatile float ap_power = 0.0;
volatile float power_factor = 0.0;
volatile float power_frequency = 0.0;

volatile float total_engery = 0.0;

static uint8_t *data_rec_p;
// 重置电能计量位
static int is_clear_total_engery = 0;
static volatile int uart_tx_rx_timestamp = 0;
extern volatile int debug;

// 循环开关
static int loop = 1;
// 循环计数
static volatile int loop_count = 0;
// 开机默认5秒一次循环
static int rec_wait = 5;

// 串口初始化
void uart_init()
{
	lcd_print("uart init . . .");
	uart_config_t uart_config = {
		.baud_rate = 9600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};

	// Set UART log level
	esp_log_level_set(TAG, ESP_LOG_INFO);

	ESP_LOGI(TAG, "Start uart configure.");

	// Install UART driver (we don't need an event queue here)
	// In this example we don't even use a buffer for sending data.
	ESP_ERROR_CHECK(uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));

	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));

	ESP_LOGI(TAG, "UART set pins, mode and install driver.");

	// Set UART pins as per KConfig settings
	ESP_ERROR_CHECK(uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

	// Set RS485 half duplex mode
	ESP_ERROR_CHECK(uart_set_mode(uart_num, UART_MODE_RS485_HALF_DUPLEX));

	// Set read timeout of UART TOUT feature
	ESP_ERROR_CHECK(uart_set_rx_timeout(uart_num, ECHO_READ_TOUT));

	// Allocate buffers for UART rec
	data_rec_p = (uint8_t *)malloc(sizeof(uint8_t) * BUF_SIZE);

	lcd_print("watchdog init . . .");
	// 看门狗初始化
	task_watchdog_init();
}

/*
 *串口发送信息*/
/*
static void uart_send(char data_p[], int len)
{

	int crc = crc_cal(data_p, len - 2);
	data_p[len - 2] = crc & 0xFF;		   // 校验位低
	data_p[len - 1] = (crc & 0xFF00) >> 8; // 校验位高

	ESP_LOGI(TAG, "UART send len: %d", len);
	if (debug)
	{
		printf("crc: 0x%02X, 0x%02X \r\n", crc & 0xFF, (crc & 0xFF00) >> 8);
		ESP_LOGI(TAG, "UART send data:\r\n");

		for (size_t i = 0; i < len; i++)
		{
			printf("0x%02x ", (uint8_t)data_p[i]);
		}
		printf("\r\n");
	}
	if (uart_write_bytes(uart_num, data_p, len) != len)
	{
		ESP_LOGE(TAG, "Send data critical failure.");
		// add your code to handle sending failure here
		abort();
	}
}
*/
/*
 *串口接受信息*/
/*
static int uart_rec(uint8_t *data_rec_p)
{
	// Read data from UART
	return uart_read_bytes(uart_num, data_rec_p, sizeof(uint8_t) * BUF_SIZE, PACKET_READ_TICS);
}
*/
/*
 *删除串口循环任务*/
void remove_rec_task()
{
	loop = 0;
}

/*
 *更改循环等待时间*/
void change_rec_wait(int sec)
{

	loop_count = 0;
	rec_wait = sec;
}
/*
 *清除累计电量*/
void set_is_clear_total_engery()
{
	is_clear_total_engery = 1;
}

int get_loop_count()
{
	return loop_count;
}

int get_rec_wait()
{
	return rec_wait;
}

int get_uart_tx_rx_timestamp()
{
	return uart_tx_rx_timestamp;
}
