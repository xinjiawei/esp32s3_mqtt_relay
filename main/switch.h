#pragma once
#include "gpio_init.h"

enum switch_states{
	switch_close,
	switch_open,
	switch_toggle
};

void switch_control1(int pin_no, enum switch_states state);
void switch_control2(int pin_no, enum switch_states state);