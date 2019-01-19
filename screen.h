#pragma once

#include <stdbool.h>

#define SCREEN_WIN_TITLE "CHIP8"
#define SCREEN_DEFAULT_SCALE 3

void screen_init(size_t);
void screen_cls(void);
void screen_destroy(void);
bool screen_draw(uint8_t, uint8_t, uint8_t, uint8_t[]);