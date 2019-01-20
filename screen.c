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
#define PX_SZ 0x1 << px_scale

#define SET_COLOR(C) SDL_SetRenderDrawColor(ren, C[0], C[1], C[2], C[3])
#define IDX(X, Y) SCREEN_W * (Y) + (X)

static bool vmem[SCREEN_W * SCREEN_H] = {0};
static SDL_Window *win = 0;
static SDL_Renderer *ren = 0;
static size_t px_scale = 0;
static uint8_t bg[] = DEFAULT_BG;
static uint8_t fg[] = DEFAULT_FG;

void screen_init(size_t scale)
{
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        FAIL(SDL_GetError());
    }
    px_scale = (scale) ? scale : SCREEN_DEFAULT_SCALE;
    win = SDL_CreateWindow(
        SCREEN_WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        0x1 << (SCREEN_W_EXP + px_scale),
        0x1 << (SCREEN_H_EXP + px_scale),
        0
    );
    if (!win) {
        FAIL(SDL_GetError());
    }
    ren = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        FAIL(SDL_GetError());
    }
    SET_COLOR(bg);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);
}

void screen_cls(void)
{
    for (size_t i = 0; i < SCREEN_H * SCREEN_W; ++i) {
        vmem[i] = false;
    }
    if (ren) {
        SET_COLOR(bg);
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }
}

uint8_t screen_draw(uint8_t x, uint8_t y, uint8_t h, uint8_t const spr[h])
{
    if (ren) {
        SET_COLOR(bg);
        SDL_RenderClear(ren);
    }
    uint8_t changed = 0x0;
    size_t xpos = x;
    for (size_t i = 0; i < h; ++i, xpos = x) {
        for (size_t j = SPRITE_W - 1; j < SPRITE_W; --j, ++xpos) {
            uint8_t sbit = (spr[i] & (0x1 << j)) >> j;
            uint8_t vbit = vmem[IDX(xpos, y + i)];
            vmem[IDX(xpos, y + i)] = sbit ^ vbit;
            changed = changed | (sbit & vbit);
        }
    }
    SET_COLOR(fg);
    for (size_t i = 0; i < SCREEN_H; ++i) {
        for (size_t j = 0; j < SCREEN_W; ++j) {
            if (vmem[IDX(j, i)]) {
                SDL_Rect r = {
                    .x = j * PX_SZ,
                    .y = i * PX_SZ,
                    .w = PX_SZ,
                    .h = PX_SZ,
                };
                SDL_RenderFillRect(ren, &r);
            }
        }
    }
    SDL_RenderPresent(ren);
    return changed;
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