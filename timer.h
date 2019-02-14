/*
    Chip8's timer system includes a general purpose timer and a sound
    timer that produces an audible beep when it ticks down to zero.
    As implemented, instead of making what would almost assuredly be
    an extremey annoying ruckus, the default behavior is just to notify
    the user via STDOUT that a sound event has occured.

    In order to function properly, timer_init needs to be called
    before any calls to timer_update. The timer_update function is
    intended to be run once per Chip8 cycle.
*/
#pragma once

#include <stdint.h>

void timer_init(void);
void timer_update(void);
void timer_set_delay(uint8_t);
uint8_t timer_get_delay(void);
void timer_set_sound(uint8_t);
uint8_t timer_get_sound(void);