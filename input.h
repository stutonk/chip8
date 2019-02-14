/*
    Chip8's input keypad is a 4x4 matrix arranged thusly:

    +---+---+---+---+
    | 1 | 2 | 3 | C |
    +---+---+---+---+
    | 4 | 5 | 6 | D |
    +---+---+---+---+
    | 7 | 8 | 9 | E |
    +---+---+---+---+
    \ A | 0 | B | F |
    +---+---+---+---+

    and correspond to the keyboard keys 1234, QWER, ASDF, ZXCV. If you
    have a layout other than QWERTY, you'll need to adjust the key
    values in input.c.

    An additional 'quit the program unconditionally` key is provided
    (Esc by default).

    Please note that the input_update function is intended to be run
    once every Chip8 cycle. The other functions allow one to
    asynchronosly check whether a specific key or which key, if any,
    is currently being pressed at the time of the function call.
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>

void input_update(void);
bool input_query(uint8_t);
uint8_t input_get_key(void);