#include "switch.h"

#include "mqtt4.h"
#include "buzzer.h"
#include "led.h"
#include "lcd.h"

// default is 0
enum switch_states swith_state1 = switch_close;
enum switch_states swith_state2 = switch_close;

void switch_control1(int pin_no,enum switch_states state) {
	swith_state1 = state == switch_toggle ? (swith_state1 ? switch_close : switch_open) : state;
	gpio_set_level(pin_no, swith_state1);

	mqtt_app_publish("s3sw1_state", swith_state1 ? "sw11":"sw10");
	led_blink(0, 0, 16);
	buzzer();
	char *temp = (char *)malloc(20);
	sprintf(temp, "sw1 %d, sw2 %d", swith_state1, swith_state2);
	lcd_print(temp);
	free(temp);
}

void switch_control2(int pin_no, enum switch_states state)
{
	swith_state2 = state == switch_toggle ? (swith_state2 ? switch_close : switch_open) : state;
	gpio_set_level(pin_no, swith_state2);

	mqtt_app_publish("s3sw2_state", swith_state2 ? "sw21" : "sw20");
	led_blink(0, 0, 16);
	buzzer();
	char *temp = (char *)malloc(20);
	sprintf(temp, "sw1 %d, sw2 %d", swith_state1, swith_state2);
	lcd_print(temp);
	free(temp);
}
