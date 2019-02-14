/*
    The Chip8 screen is normally a tiny 64x32 so screen_init includes a
    scale paremeter by which these dimensions are multiplied.

    The drawing function takes an x,y location (whose origin is in the
    upper left corner) as well as a height and array containing the
    sprite data which must be height bytes in length (all Chip8 sprites
    are implicitly 8 pixels wide).
    
    Drawing over the bottom or right side of the screen simply wraps 
    around to the top or left side, respectively.
*/
#pragma once

#include <stdint.h>

#define SCREEN_WIN_TITLE "CHIP8"
#define SCREEN_DEFAULT_SCALE 3

void screen_init(size_t);
void screen_cls(void);
void screen_destroy(void);
uint8_t screen_draw(uint8_t, uint8_t, uint8_t, const uint8_t[]);