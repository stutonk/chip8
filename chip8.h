#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>


#define CHIP8_KEY_QUIT SDLK_ESCAPE
/* Assuming QWERTY layout */
#define CHIP8_NUMKEYS 16
#define CHIP8_KEY_1 SDLK_1
#define CHIP8_KEY_2 SDLK_2
#define CHIP8_KEY_3 SDLK_3
#define CHIP8_KEY_C SDLK_4
#define CHIP8_KEY_4 SDLK_q
#define CHIP8_KEY_5 SDLK_w
#define CHIP8_KEY_6 SDLK_e
#define CHIP8_KEY_D SDLK_r
#define CHIP8_KEY_7 SDLK_a
#define CHIP8_KEY_8 SDLK_s
#define CHIP8_KEY_9 SDLK_d
#define CHIP8_KEY_E SDLK_f
#define CHIP8_KEY_A SDLK_z
#define CHIP8_KEY_0 SDLK_x
#define CHIP8_KEY_B SDLK_c
#define CHIP8_KEY_F SDLK_v

#define CHIP8_ADDR uint16_t
#define CHIP8_MEM_SZ 4096 / sizeof(CHIP8_ADDR)
#define CHIP8_STACK_SZ 50

void chip8_init(size_t);
void chip8_reset(void);
void chip8_destroy(void);
bool chip8_load(uint16_t, CHIP8_ADDR[], size_t);
void chip8_execute(CHIP8_ADDR);