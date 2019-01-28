#pragma once

#include <stdint.h>

void timer_init(void);
void timer_update(void);
void timer_set_delay(uint8_t);
uint8_t timer_get_delay(void);
void timer_set_sound(uint8_t);
uint8_t timer_get_sound(void);