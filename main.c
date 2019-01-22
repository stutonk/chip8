#include <stdio.h>
#include "chip8.h"
#include "util.h"

static CHIP8_ADDR loop[] = {0xe0a1, 0x1fff, 0x1000};

int main(int argc, char *argv[argc+1])
{
    if (SDL_Init(0) != 0) {
        FAIL(SDL_GetError());
    }
    atexit(SDL_Quit);
    /* getopt for scale, entry point */
    chip8_init(0);
    if (argc > 1) {
        uint16_t buf[CHIP8_MEM_SZ] = {0};
        FILE *in = fopen(argv[1], "rb");
        size_t bytes_in = fread(buf, sizeof(CHIP8_ADDR), CHIP8_MEM_SZ, in);
        if (!feof(in)) {
            FAIL("input file overflows available program memory");
        }
        chip8_load(0, buf, bytes_in);
    } else {
        puts("warning: no program");
        chip8_load(0, loop, sizeof(loop) / sizeof(CHIP8_ADDR));
    }
    chip8_execute(0);
    chip8_destroy();
}