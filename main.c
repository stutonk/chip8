#include <stdio.h>
#include "chip8.h"

#define BUF_SZ CHIP8_MEM_SZ-CHIP8_PRG_MEM_END

static uint8_t loop[] = {0x10,0x00};

int main(int argc, char *argv[argc+1])
{
    /* getopt for scale, entry point */
    if (argc > 1) {
        uint8_t buf[BUF_SZ] = {0};
        FILE *in = fopen(argv[1], "rb");
        size_t bytes_in = fread(buf, sizeof(uint8_t), BUF_SZ, in);
        if (!feof(in)) {
            fprintf(
                stderr, 
                "main() error: file %s overflows available program memory\n", 
                argv[1]
            );
            return EXIT_FAILURE;
        }
        chip8_load(0, buf, bytes_in);
    } else {
        puts("warning: no program");
        chip8_load(0, loop, 2);
    }
    chip8_execute(0);
    chip8_destroy();
}