#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "screen.h"

#define INSTR_SZ 2
#define NUMREGS 16

#define WIN_TITLE "CHIP8"
#define DEFAULT_SCALE 3
#define SCREEN_W 64
#define SCREEN_W_EXP 6
#define SCREEN_H 32
#define SCREEN_H_EXP 5
#define SPRITE_W 8

static uint8_t mem[CHIP8_MEM_SZ] = {0};
static uint8_t reg_v[NUMREGS] = {0};
static uint16_t reg_i = 0;
static uint16_t pc = 0;
static uint16_t sp = CHIP8_STACK_BOTTOM;
static bool vid_mem[SCREEN_W * SCREEN_H] = {0};
static SDL_Window *win = 0;
static SDL_Renderer *ren = 0;

static void cleanup_SDL(void);
static void fail(char const *, char const *);
static void cls(void);
static void draw(uint8_t, uint8_t, uint8_t);
static void key(uint8_t, uint8_t);
static void op_0(uint8_t);
static void op_8(uint8_t, uint8_t, uint8_t);
static void op_f(uint8_t, uint8_t);

void chip8_init(size_t scale) 
{
    scale = (scale) ? scale : DEFAULT_SCALE;
    chip8_reset();
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0) {
        fail("chip8_init", SDL_GetError());
    }
    win = SDL_CreateWindow(
        WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        0x1 << (SCREEN_W_EXP + scale),
        0x1 << (SCREEN_H_EXP + scale),
        0
    );
    if (!win) {
        fail("chip8_init", SDL_GetError());
    }
    ren = SDL_CreateRenderer(
        win,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!ren) {
        fail("chip8_init", SDL_GetError());
    }
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);
}

void chip8_reset(void)
{
    for (size_t i = 0; i < CHIP8_MEM_SZ; ++i) {
        mem[i] = 0;
    }
    for (size_t i = 0; i < NUMREGS; ++i) {
        reg_v[i] = 0;
    }
    reg_i = 0;
    pc = 0;
    sp = CHIP8_STACK_BOTTOM;
    cls();
}

void chip8_destroy(void) 
{
    cleanup_SDL();
}

bool chip8_load(uint16_t offset, uint8_t bytes[], size_t num)
{
    if (offset + num >= CHIP8_PRG_MEM_END) {
        return false;
    }
    memcpy(mem + offset, bytes, num);
    return true;
}

void chip8_execute(uint16_t entry)
{
    SDL_Event e = {0};
    for (pc = entry; pc < CHIP8_PRG_MEM_END; pc += INSTR_SZ) {
        if (SDL_PollEvent(&e) 
            && e.type == SDL_KEYUP 
            && e.key.keysym.sym == CHIP8_QUIT_KEY
        ) {
            break;
        }
        
        uint8_t op_hi = mem[pc];
        uint8_t op_lo = mem[pc+1];
        uint16_t addr_operand = op_lo + ((op_hi & 0x0f) << 8);
        size_t reg_operand_x = op_hi & 0x0f;
        size_t reg_operand_y = op_lo >> 4;

        switch (op_hi >> 4) {
            case 0x0: //CLS, RET
                op_0(op_lo);
                break;
            case 0x1: //JMP
                pc = addr_operand - INSTR_SZ;
                break;
            case 0x2: //CALL
                if (sp > CHIP8_STACK_TOP) {
                    fail("chip8_execute", "stack overflow");
                }
                mem[sp] = (pc >> 8);
                mem[sp+1] = pc & 0x00ff;
                sp += INSTR_SZ;
                pc = addr_operand;
                break;
            case 0x3: //JE
                if (reg_v[reg_operand_x] == op_lo) {
                    pc += INSTR_SZ;
                }
                break;
            case 0x4: //JNE
                if (reg_v[reg_operand_x] != op_lo) {
                    pc += INSTR_SZ;
                }
                break;
            case 0x5: //JRE
                if (reg_v[reg_operand_x] == reg_v[reg_operand_y]) {
                    pc += INSTR_SZ;
                }
                break;
            case 0x6: //SETV
                reg_v[reg_operand_x] = op_lo;
                break;
            case 0x7: //ADD
                reg_v[reg_operand_x] += op_lo;
                break;
            case 0x8: //Math ops
                op_8(op_lo & 0x0f, reg_operand_x, reg_operand_y);
                break;
            case 0x9: //JRNE
                if (reg_v[reg_operand_x] != reg_v[reg_operand_y]) {
                    pc += INSTR_SZ;
                }
                break;
            case 0xa: //SETI
                reg_i = addr_operand;
                break;
            case 0xb: //JR0
                pc = addr_operand + reg_v[0] - INSTR_SZ;
            case 0xc: //VRND
                reg_v[reg_operand_x] = op_lo & (rand() % 0xff);
                break;
            case 0xd://DRAW
                draw(reg_operand_x, reg_operand_y, op_lo & 0x0f);
                break;
            case 0xe: //Skip if [not] key
                key(op_lo, reg_operand_x);
                break;
            case 0xf: //Misc ops
                op_f(op_lo, reg_operand_x);
                break;
            default:
                fail("chip8_execute", "illegal instruction");
                break;
        }
    }
}

static
void cleanup_SDL(void) 
{
    if (win) {
        SDL_DestroyWindow(win);
    }
    if (ren) {
        SDL_DestroyRenderer(ren);
    }
    SDL_Quit();
}

static
void fail(char const *func_name, char const *reason)
{
    cleanup_SDL();
    fprintf(stderr, "%s() error: %s\n", func_name, reason);
    exit(EXIT_FAILURE);
}

static
void cls(void)
{
    for (size_t i = 0; i < SCREEN_H*SCREEN_W; ++i) {
        vid_mem[i] = 0;
    }
    if (ren) {
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
    }
}

static 
void draw(uint8_t x, uint8_t y, uint8_t w) {}

static inline
void key(uint8_t op, uint8_t reg) {}

static inline
void op_0(uint8_t op) 
{
    switch (op) {
        case 0xe0: //CLS
            cls();
            break;
        case 0xee: //RET
            pc = mem[sp] - INSTR_SZ;
            sp -= INSTR_SZ;
            break;
        default:
            fail("op_0", "illegal instruction");
    }
}

static inline
void op_8(uint8_t op, uint8_t x, uint8_t y) 
{
    fail("op_8", "unimplemented");
}

static inline
void op_f(uint8_t op, uint8_t x) 
{
    fail("op_f", "unimplemented");
}