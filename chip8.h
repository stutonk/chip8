#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>

#define CHIP8_QUIT_KEY SDLK_ESCAPE
#define CHIP8_MEM_SZ 4096
#define CHIP8_STACK_SZ 50

void chip8_init(size_t);
void chip8_reset(void);
void chip8_destroy(void);
bool chip8_load(uint16_t, uint8_t[], size_t);
void chip8_execute(uint16_t);