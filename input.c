#include <stdlib.h>
#include <SDL.h>
#include "chip8.h"

#define NUMKEYS 16
#define KEY_1 SDLK_1
#define KEY_2 SDLK_2
#define KEY_3 SDLK_3
#define KEY_C SDLK_4
#define KEY_4 SDLK_q
#define KEY_5 SDLK_w
#define KEY_6 SDLK_e
#define KEY_D SDLK_r
#define KEY_7 SDLK_a
#define KEY_8 SDLK_s
#define KEY_9 SDLK_d
#define KEY_E SDLK_f
#define KEY_A SDLK_z
#define KEY_0 SDLK_x
#define KEY_B SDLK_c
#define KEY_F SDLK_v
#define KEY_QUIT SDLK_ESCAPE

#define CASE_KEY_RETURN(N) \
    case KEY_ ## N:        \
        return 0x ## N

#define QUIT               \
    chip8_destroy();       \
    exit(EXIT_SUCCESS)

static uint8_t key_to_num(SDL_Keycode);

static uint16_t Key = 0;
static SDL_Event E = {0};

void input_update()
{
    if (SDL_PollEvent(&E)) {
        if (E.type == SDL_KEYDOWN) {
            uint8_t keynum = key_to_num(E.key.keysym.sym);
            if (keynum < NUMKEYS) {
                Key |= 1 << keynum;
            }
        } else if (E.type == SDL_KEYUP) {
            if (E.key.keysym.sym == KEY_QUIT) {
                QUIT;
            }
            uint8_t keynum = key_to_num(E.key.keysym.sym);
            if (keynum < NUMKEYS) {
                Key &= 0 << keynum;
            }
        }
        else if (E.type == SDL_QUIT) {
            QUIT;
        }
    }
}

bool input_query(uint8_t key)
{
    return (Key & 1 << key) ? true : false;
}

uint8_t input_get_key()
{
    while (SDL_WaitEvent(&E)) {
        if (E.type == SDL_KEYUP) {
            uint8_t keynum = key_to_num(E.key.keysym.sym);
            if (E.key.keysym.sym == KEY_QUIT) {
                QUIT;
            } else if (keynum < NUMKEYS) {
                return keynum;
            }
        } else if (E.type == SDL_QUIT) {
            QUIT;
        }
    }
    return 0;
}

static uint8_t key_to_num(SDL_Keycode key) {
    switch (key) {
        CASE_KEY_RETURN(0);
        CASE_KEY_RETURN(1);
        CASE_KEY_RETURN(2);
        CASE_KEY_RETURN(3);
        CASE_KEY_RETURN(4);
        CASE_KEY_RETURN(5);
        CASE_KEY_RETURN(6);
        CASE_KEY_RETURN(7);
        CASE_KEY_RETURN(8);
        CASE_KEY_RETURN(9);
        CASE_KEY_RETURN(A);
        CASE_KEY_RETURN(B);
        CASE_KEY_RETURN(C);
        CASE_KEY_RETURN(D);
        CASE_KEY_RETURN(E);
        CASE_KEY_RETURN(F);
    }
    return NUMKEYS;
}