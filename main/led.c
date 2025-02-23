#include "led.h"

#include "lcd.h"

static const char *TAG = "led";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

#define WS2812_LED 1

#ifdef WS2812_LED

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM 48
rmt_channel_handle_t led_chan = NULL;
rmt_encoder_handle_t simple_encoder = NULL;

static uint8_t led_strip_pixels[3];

static const rmt_symbol_word_t ws2812_zero = {
	.level0 = 1,
	.duration0 = 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T0H=0.3us
	.level1 = 0,
	.duration1 = 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T0L=0.9us
};

static const rmt_symbol_word_t ws2812_one = {
	.level0 = 1,
	.duration0 = 0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T1H=0.9us
	.level1 = 0,
	.duration1 = 0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000, // T1L=0.3us
};

// reset defaults to 50uS
static const rmt_symbol_word_t ws2812_reset = {
	.level0 = 1,
	.duration0 = RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2,
	.level1 = 0,
	.duration1 = RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2,
};

static size_t encoder_callback(const void *data, size_t data_size,
							   size_t symbols_written, size_t symbols_free,
							   rmt_symbol_word_t *symbols, bool *done, void *arg)
{
	// We need a minimum of 8 symbol spaces to encode a byte. We only
	// need one to encode a reset, but it's simpler to simply demand that
	// there are 8 symbol spaces free to write anything.
	if (symbols_free < 8)
	{
		return 0;
	}

	// We can calculate where in the data we are from the symbol pos.
	// Alternatively, we could use some counter referenced by the arg
	// parameter to keep track of this.
	size_t data_pos = symbols_written / 8;
	uint8_t *data_bytes = (uint8_t *)data;
	if (data_pos < data_size)
	{
		// Encode a byte
		size_t symbol_pos = 0;
		for (int bitmask = 0x80; bitmask != 0; bitmask >>= 1)
		{
			if (data_bytes[data_pos] & bitmask)
			{
				symbols[symbol_pos++] = ws2812_one;
			}
			else
			{
				symbols[symbol_pos++] = ws2812_zero;
			}
		}
		// We're done; we should have written 8 symbols.
		return symbol_pos;
	}
	else
	{
		// All bytes already are encoded.
		// Encode the reset, and we're done.
		symbols[0] = ws2812_reset;
		*done = 1; // Indicate end of the transaction.
		return 1;  // we only wrote one symbol
	}
}

static void blink_ws2812(int g, int r, int b)
{
	ESP_LOGI(TAG, "Start LED chase");
	rmt_transmit_config_t tx_config = {
		.loop_count = 0, // no transfer loop
	};

	led_strip_pixels[0] = g;
	led_strip_pixels[1] = r;
	led_strip_pixels[2] = b;
	// Flush RGB values to LEDs
	ESP_ERROR_CHECK(rmt_transmit(led_chan, simple_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
	ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
}

#else
#define BLINK_GPIO 8

static void blink_led(const uint32_t s_led_state)
{
	/* Set the GPIO level according to the state (LOW or HIGH)*/
	gpio_set_level(BLINK_GPIO, s_led_state);
}

#endif // WS2812_LED

/*
 *led初始化*/
void led_configure(void)
{
	lcd_print("led init . . .");
#ifdef WS2812_LED

	ESP_LOGI(TAG, "Create RMT TX channel");
	rmt_tx_channel_config_t tx_chan_config = {
		.clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
		.gpio_num = RMT_LED_STRIP_GPIO_NUM,
		.mem_block_symbols = 64, // increase the block size can make the LED less flickering
		.resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
		.trans_queue_depth = 4, // set the number of transactions that can be pending in the background
	};
	ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

	ESP_LOGI(TAG, "Create simple callback-based encoder");
	const rmt_simple_encoder_config_t simple_encoder_cfg = {
		.callback = encoder_callback
		// Note we don't set min_chunk_size here as the default of 64 is good enough.
	};
	ESP_ERROR_CHECK(rmt_new_simple_encoder(&simple_encoder_cfg, &simple_encoder));

	ESP_LOGI(TAG, "Enable RMT TX channel");
	ESP_ERROR_CHECK(rmt_enable(led_chan));
	
	#else
	
	ESP_LOGI(TAG, "configured to blink GPIO LED!");
	gpio_reset_pin(BLINK_GPIO);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
	
	#endif // WS2812_LED
}

/*
 *led闪烁*/
void led_blink(int g, int r, int b)
{
	#ifdef WS2812_LED
	blink_ws2812(g,r,b);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	blink_ws2812(0,0,0);

#else

	blink_led(0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	blink_led(1);

	#endif // WS2812_LED
}