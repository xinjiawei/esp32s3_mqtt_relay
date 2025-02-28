#include "button.h"
#include "buzzer.h"
#include "switch.h"

#define SHORT_PRESS_TIME_MS 1000 // 短按时间阈值
#define LONG_PRESS_TIME_MS_MIN 2000	 // 长按时间阈值
#define LONG_PRESS_TIME_MS_MAX 5000	 // 长按时间阈值
#define DEBOUNCE_TIME_MS 10		 // 消抖时间阈值

static const char *TAG = "BUTTON";
// 定义按钮事件类型
typedef enum
{
	BUTTON_EVENT_PRESSED, // 按钮按下
	BUTTON_EVENT_RELEASED // 按钮释放
} button_event_t;

// 定义队列句柄
static QueueHandle_t button_event_queue = NULL;
button_event_t button_event_last = BUTTON_EVENT_RELEASED;

// GPIO 中断服务程序
static void IRAM_ATTR button_isr_handler(void *arg)
{
	static TickType_t last_interrupt_time = 0; // 上一次中断时间
	static TickType_t press_time = 0;		   // 按钮按下时间
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	button_event_t button_event_now;

	// 获取当前时间
	TickType_t current_time = xTaskGetTickCount();

	// 消抖逻辑：如果两次中断的时间间隔小于消抖时间，忽略本次中断
	if (current_time - last_interrupt_time < pdMS_TO_TICKS(DEBOUNCE_TIME_MS))
	{
		return;
	}
	last_interrupt_time = current_time;

	if (gpio_get_level(BUTTON_GPIO_PIN) == 0 && button_event_last == BUTTON_EVENT_RELEASED)
	{							   // 按钮按下（假设按下为低电平）
		press_time = current_time; // 记录按下时间
		button_event_now = BUTTON_EVENT_PRESSED;
		button_event_last = BUTTON_EVENT_PRESSED;
	}
	else
	{ // 按钮释放
		button_event_now = BUTTON_EVENT_RELEASED;
		button_event_last = BUTTON_EVENT_RELEASED;
	}

	// 发送事件到队列
	xQueueSendFromISR(button_event_queue, &button_event_now, &xHigherPriorityTaskWoken);

	// 如果需要，触发任务切换
	if (xHigherPriorityTaskWoken)
	{
		portYIELD_FROM_ISR();
	}
}

static int button_match = 0;

// 按钮处理任务
static void button_task(void *arg)
{
	button_event_t event;
	TickType_t press_time = 0;
	TickType_t release_time, press_duration = 0;
	while (1)
	{
		// 从队列中接收按钮事件
		if (xQueueReceive(button_event_queue, &event, portMAX_DELAY) == pdTRUE)
		{
			// 效果不好，不准确，弃用
			/*
			if (event == BUTTON_EVENT_PRESSED)
			{
				++button_match;
			}
			else
			{
				++button_match;
			}
*/
			++button_match;
			// 这种效果好
			if (button_match % 2 == 1)
			{
				press_time = xTaskGetTickCount(); // 记录按下时间
			}
			else
			{
				// 计算按下时间
				release_time = xTaskGetTickCount();
				press_duration = release_time - press_time;

				// 判断按下时间
				if (press_duration > 5 && press_duration < pdMS_TO_TICKS(SHORT_PRESS_TIME_MS))
				{
					
					// buzzer();
					// led_blink(0, 16, 0);
					ESP_LOGI(TAG, "Power trager %d %lu %lu", button_match, press_time, press_duration);
					switch_control1(SWITCH_GPIO_PIN1, switch_close);
					switch_control2(SWITCH_GPIO_PIN2, switch_close);
					// 短按逻辑：电源停机
				}
				else if (press_duration >= pdMS_TO_TICKS(LONG_PRESS_TIME_MS_MIN) && press_duration <= pdMS_TO_TICKS(LONG_PRESS_TIME_MS_MAX))
				{
					ESP_LOGI(TAG, "Factory trager");
					// 长按逻辑：恢复出厂设置
				}
			}
		}
	}
}

void button_init(void)
{

	// 创建按钮事件队列
	button_event_queue = xQueueCreate(10, sizeof(button_event_t));
	if (button_event_queue == NULL)
	{
		ESP_LOGE(TAG,"Failed to create queue!\n");
		return;
	}

	// 安装 GPIO 中断服务
	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON_GPIO_PIN, button_isr_handler, NULL);

	// 创建按钮处理任务
	xTaskCreate(button_task, "button_task", 3000, NULL, 10, NULL);

	ESP_LOGI(TAG,"Button task started");
}