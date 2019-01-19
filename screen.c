#include <SDL.h>
#include "screen.h"
#include "util.h"

#define SCREEN_W 64
#define SCREEN_W_EXP 6
#define SCREEN_H 32
#define SCREEN_H_EXP 5
#define SPRITE_W 8

static bool vmem[SCREEN_W * SCREEN_H] = {0};
static SDL_Window *win = 0;
static SDL_Renderer *ren = 0;

void screen_init(size_t scale)
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    win = SDL_CreateWindow(
        SCREEN_WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        0x1 << (SCREEN_W_EXP + scale),
        0x1 << (SCREEN_H_EXP + scale),
        0
    );
    if (!win) {
        FAIL("chip8_init", SDL_GetError());
    }
    ren = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        FAIL("chip8_init", SDL_GetError());
    }
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);
}

void screen_cls(void)
{
    for (size_t i = 0; i < SCREEN_H * SCREEN_W; ++i) {
        vmem[i] = false;
    }
    if (ren) {
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }
}

void screen_draw(uint8_t x, uint8_t y, uint8_t h)
{
    
}

void screen_destroy(void)
{
    if (win) {
        SDL_DestroyWindow(win);
    }
    if (ren) {
        SDL_DestroyRenderer(ren);
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}