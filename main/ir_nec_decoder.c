#include "ir_nec_decoder.h"
#include "switch.h"

#define EXAMPLE_IR_NEC_DECODE_MARGIN 200 // Tolerance for parsing RMT symbols into bit stream

/**
 * @brief NEC timing spec
 */
#define NEC_LEADING_CODE_DURATION_0 9000
#define NEC_LEADING_CODE_DURATION_1 4500
#define NEC_PAYLOAD_ZERO_DURATION_0 560
#define NEC_PAYLOAD_ZERO_DURATION_1 560
#define NEC_PAYLOAD_ONE_DURATION_0 560
#define NEC_PAYLOAD_ONE_DURATION_1 1690
#define NEC_REPEAT_CODE_DURATION_0 9000
#define NEC_REPEAT_CODE_DURATION_1 2250

/**
 * @brief Saving NEC decode results
 */
static uint16_t s_nec_code_address;
static uint16_t s_nec_code_command;

static const char *TAG = "IR_DEC";

/**
 * @brief Check whether a duration is within expected range
 */
static inline bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration)
{
	return (signal_duration < (spec_duration + EXAMPLE_IR_NEC_DECODE_MARGIN)) &&
		   (signal_duration > (spec_duration - EXAMPLE_IR_NEC_DECODE_MARGIN));
}

/**
 * @brief Check whether a RMT symbol represents NEC logic zero
 */
static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols)
{
	return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
		   nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

/**
 * @brief Check whether a RMT symbol represents NEC logic one
 */
static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols)
{
	return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
		   nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}

/**
 * @brief Decode RMT symbols into NEC address and command
 */
static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols)
{
	rmt_symbol_word_t *cur = rmt_nec_symbols;
	uint16_t address = 0;
	uint16_t command = 0;
	bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
							  nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);
	if (!valid_leading_code)
	{
		return false;
	}
	cur++;
	for (int i = 0; i < 16; i++)
	{
		if (nec_parse_logic1(cur))
		{
			address |= 1 << i;
		}
		else if (nec_parse_logic0(cur))
		{
			address &= ~(1 << i);
		}
		else
		{
			return false;
		}
		cur++;
	}
	for (int i = 0; i < 16; i++)
	{
		if (nec_parse_logic1(cur))
		{
			command |= 1 << i;
		}
		else if (nec_parse_logic0(cur))
		{
			command &= ~(1 << i);
		}
		else
		{
			return false;
		}
		cur++;
	}
	// save address and command
	s_nec_code_address = address;
	s_nec_code_command = command;
	return true;
}

/**
 * @brief Check whether the RMT symbols represent NEC repeat code
 */
static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols)
{
	return nec_check_in_range(rmt_nec_symbols->duration0, NEC_REPEAT_CODE_DURATION_0) &&
		   nec_check_in_range(rmt_nec_symbols->duration1, NEC_REPEAT_CODE_DURATION_1);
}


/**
 * @brief Decode RMT symbols into NEC scan code and print the result
 */
void example_parse_nec_frame(rmt_symbol_word_t *rmt_nec_symbols, size_t symbol_num)
{
	ESP_LOGI(TAG, "NEC frame start---");
	for (size_t i = 0; i < symbol_num; i++)
	{
		ESP_LOGI(TAG, "{%d:%d},{%d:%d}", rmt_nec_symbols[i].level0, rmt_nec_symbols[i].duration0,
			   rmt_nec_symbols[i].level1, rmt_nec_symbols[i].duration1);
	}
	ESP_LOGI(TAG, "---NEC frame end: ");
	// decode RMT symbols
	switch (symbol_num)
	{
	case 34: // NEC normal frame
		if (nec_parse_frame(rmt_nec_symbols))
		{
			ESP_LOGI(TAG, "Address=%04X, Command=%04X", s_nec_code_address, s_nec_code_command);
		}

		switch (s_nec_code_command)
		{
		case 0xba45:
			switch_control1(SWITCH_GPIO_PIN1, switch_open);
			break;
		case 0xb946:
			switch_control1(SWITCH_GPIO_PIN1, switch_close);
			break;
		case 0xb847:
			switch_control1(SWITCH_GPIO_PIN1, switch_toggle);
			break;
		case 0xbb44:
			switch_control2(SWITCH_GPIO_PIN2, switch_open);
			break;
		case 0xbf40:
			switch_control2(SWITCH_GPIO_PIN2, switch_close);
			break;
		case 0xbc43:
			switch_control2(SWITCH_GPIO_PIN2, switch_toggle);
			break;
		}

			break;
	case 2: // NEC repeat frame
		if (nec_parse_frame_repeat(rmt_nec_symbols))
		{
			ESP_LOGI(TAG, "Address=%04X, Command=%04X, repeat", s_nec_code_address, s_nec_code_command);
		}
		break;
	default:
		ESP_LOGI(TAG, "symbol_num %d, Unknown NEC frame", symbol_num);
		break;
	}
}

bool example_rmt_rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data)
{
	BaseType_t high_task_wakeup = pdFALSE;
	QueueHandle_t receive_queue = (QueueHandle_t)user_data;
	// send the received RMT symbols to the parser task
	xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
	return high_task_wakeup == pdTRUE;
}