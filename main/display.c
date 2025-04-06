
#include "display.h"

void display_print(char *messgae) {
	//lcd_print(messgae);
}

void eigth_led_display(uint8_t d1, uint8_t d2) {
	send_2_digits(d1,d2);
}