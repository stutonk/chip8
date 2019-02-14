#include <stdio.h>
#include <unistd.h>
#include "chip8.h"
#include "util.h"

/* prints "NO PROGRAM\nPRESS ESC" and loops forever */
static const uint8_t no_prog[] = {
    0x10, 0x2b, 0x90, 0xd0, 0xb0, 0x90, 0x90, 0xe0, 0x90, 0xe0, 0x80, 0x80,
    0xe0, 0x90, 0xe0, 0xa0, 0x90, 0x60, 0x90, 0x80, 0xb0, 0x60, 0xb0, 0xf0,
    0xd0, 0x90, 0x90, 0xd0, 0x15, 0x70, 0x05, 0x00, 0xee, 0x71, 0x07, 0x60,
    0x00, 0x00, 0xee, 0x70, 0x05, 0x00, 0xee, 0x61, 0x09, 0x60, 0x08, 0x64,
    0x0a, 0x65, 0x0e, 0x66, 0x05, 0x67, 0x0c, 0xa0, 0x02, 0x20, 0x1b, 0xf3,
    0x29, 0x20, 0x1b, 0x20, 0x27, 0xa0, 0x07, 0x20, 0x1b, 0xa0, 0x0c, 0x20,
    0x1b, 0xf3, 0x29, 0x20, 0x1b, 0xa0, 0x11, 0x20, 0x1b, 0xa0, 0x0c, 0x20,
    0x1b, 0xf4, 0x29, 0x20, 0x1b, 0xa0, 0x16, 0x20, 0x1b, 0x20, 0x21, 0x60,
    0x0a, 0xa0, 0x07, 0x20, 0x1b, 0xa0, 0x0c, 0x20, 0x1b, 0xf5, 0x29, 0x20,
    0x1b, 0xf6, 0x29, 0x20, 0x1b, 0x20, 0x1b, 0x20, 0x27, 0xf5, 0x29, 0x20,
    0x1b, 0xf6, 0x29, 0x20, 0x1b, 0xf7, 0x29, 0x20, 0x1b, 0x10, 0x81
};

int main(int argc, char *argv[argc+1])
{
    extern char *optarg;
    extern int optind, optopt;
    int opt = 0;
    size_t scale = 0;
    uint16_t entry = CHIP8_DEFAULT_ENTRY;
    

    while ((opt = getopt(argc, argv, ":e:s:")) != -1) {
        switch (opt) {
            case 'e':
                {
                    long earg = 0;
                    if (optarg[0] == '0') {
                        earg = strtol(optarg, NULL, 0);
                    } else {
                        earg = strtol(optarg, NULL, 10);
                    }
                    if (earg > CHIP8_MEM_SZ - 2 || earg < 0) {
                        FAIL("illegal entry address");
                    }
                    entry = earg;
                }
                break;
            case 's':
                scale = atoi(optarg);
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                goto usage;
            case '?':
                fprintf(stderr, "Unrecognized option `%c.\n", optopt);
                goto usage;
        }
    }

    if (SDL_Init(0) != 0) {
        FAIL(SDL_GetError());
    }
    atexit(SDL_Quit);
    chip8_init(scale);

    if (optind < argc) {
        uint8_t buf[CHIP8_MEM_SZ] = {0};
        FILE *in = fopen(argv[optind], "rb");
        size_t bytes_in = fread(
            buf,
            sizeof(uint8_t),
            CHIP8_MEM_SZ - entry,
            in
        );
        // possible off-by-one error
        if (!feof(in)) {
            FAIL("input file overflows available program memory");
        }
        chip8_load(entry, buf, bytes_in);
    } else {
        chip8_load(0, no_prog, sizeof(no_prog));
        entry = 0;
    }
    chip8_execute(entry);
    chip8_destroy();
    return EXIT_SUCCESS;
usage:
    printf("Usage: %s [-s scale] [-e entry_point] path/to/chip8/rom\n", 
            argv[0]);
    return EXIT_FAILURE;
}