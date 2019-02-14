/*
    The Chip8 computer consists of a binary opcode interpreter, screen
    device, input keypad, and timer subsystem. Each are implemented 
    (and documented) in their own respective translation units.
    
    See chup8.c for a list of Chip8 binary opcodes.

    Both the init and load functions allow the specification of an
    address in Chip8 memory into which to load a program as well as the
    program's entry point, respectively. Please note that most older
    Chip8 programs are hard-coded with address refrences relative to a
    certain specific value (defined here as CHIP8_DEFAULT_ENTRY). The
    default behavior of the main program should be to load and execute
    programs at this address.
*/
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL.h>

#define CHIP8_MEM_SZ 4096
#define CHIP8_STACK_SZ 24
#define CHIP8_DEFAULT_ENTRY 0x200

void chip8_init(size_t);
void chip8_reset(void);
void chip8_destroy(void);
bool chip8_load(uint16_t, const uint8_t[], size_t);
void chip8_execute(uint16_t);