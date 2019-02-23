#include <stdbool.h>
#include <SDL.h>
#include "screen.h"
#include "util.h"

#define SPRITE_W 8
#define SCREEN_W 64
#define SCREEN_W_EXP 6
#define SCREEN_H 32
#define SCREEN_H_EXP 5
#define DEFAULT_BG {0x00, 0x00, 0x00, 0xff}
#define DEFAULT_FG {0xff, 0xff, 0xff, 0xff}
#define PX_SZ 0x1 << Px_scale

#define SET_COLOR(C) SDL_SetRenderDrawColor(Ren, C[0], C[1], C[2], C[3])

static bool Vmem[SCREEN_H][SCREEN_W] = {0};
static SDL_Window *Win = 0;
static SDL_Renderer *Ren = 0;
static size_t Px_scale = 0;
static uint8_t Bg[] = DEFAULT_BG;
static uint8_t Fg[] = DEFAULT_FG;

void screen_init(size_t scale)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        FAIL(SDL_GetError());
    }
    Px_scale = (scale) ? scale : SCREEN_DEFAULT_SCALE;
    Win = SDL_CreateWindow(
        SCREEN_WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        0x1 << (SCREEN_W_EXP + Px_scale),
        0x1 << (SCREEN_H_EXP + Px_scale),
        0
    );
    if (!Win) {
        FAIL(SDL_GetError());
    }
    Ren = SDL_CreateRenderer(
        Win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!Ren) {
        FAIL(SDL_GetError());
    }
    SET_COLOR(Bg);
    SDL_RenderClear(Ren);
    SDL_RenderPresent(Ren);
}

void screen_cls(void)
{
    for (size_t i = 0; i < SCREEN_H; ++i) {
        for (size_t j = 0; j < SCREEN_W; ++j) {
            Vmem[i][j] = false;
        }
    }
    if (Ren) {
        SET_COLOR(Bg);
        SDL_RenderClear(Ren);
        SDL_RenderPresent(Ren);
    }
}

uint8_t screen_draw(uint8_t x, uint8_t y, uint8_t h, uint8_t const spr[h])
{
    if (Ren) {
        SET_COLOR(Bg);
        SDL_RenderClear(Ren);
    }
    uint8_t changed = 0x0;
    size_t xpos = x;
    for (size_t i = 0; i < h; ++i, xpos = x) {
        for (size_t j = SPRITE_W - 1; j < SPRITE_W; --j, ++xpos) {
            uint8_t _x = xpos % SCREEN_W;
            uint8_t _y = (y + i) % SCREEN_H;
            uint8_t sbit = (spr[i] & (0x1 << j)) >> j;
            uint8_t vbit = Vmem[_y][_x];
            Vmem[_y][_x] = sbit ^ vbit;
            changed = changed | (sbit & vbit);
        }
    }
    SET_COLOR(Fg);
    for (size_t i = 0; i < SCREEN_H; ++i) {
        for (size_t j = 0; j < SCREEN_W; ++j) {
            if (Vmem[i][j]) {
                SDL_Rect r = {
                    .x = j * PX_SZ,
                    .y = i * PX_SZ,
                    .w = PX_SZ,
                    .h = PX_SZ,
                };
                SDL_RenderFillRect(Ren, &r);
            }
        }
    }
    SDL_RenderPresent(Ren);
    return changed;
}

void screen_destroy(void)
{
    if (Win) {
        SDL_DestroyWindow(Win);
    }
    if (Ren) {
        SDL_DestroyRenderer(Ren);
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}