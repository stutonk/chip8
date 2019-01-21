#include <stdio.h>
#include "chip8.h"
#include "util.h"

#define BUF_SZ CHIP8_MEM_SZ

static uint8_t loop[] = {0x10,0x00};

int main(int argc, char *argv[argc+1])
{
    if (SDL_Init(0) != 0) {
        FAIL(SDL_GetError());
    }
    atexit(SDL_Quit);
    /* getopt for scale, entry point */
    chip8_init(0);
    if (argc > 1) {
        uint8_t buf[BUF_SZ] = {0};
        FILE *in = fopen(argv[1], "rb");
        size_t bytes_in = fread(buf, sizeof(uint8_t), BUF_SZ, in);
        if (!feof(in)) {
            FAIL("input file overflows available program memory");
        }
        chip8_load(0, buf, bytes_in);
    } else {
        puts("warning: no program");
        chip8_load(0, loop, 2);
    }
    chip8_execute(0);
    chip8_destroy();
}