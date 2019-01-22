#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "screen.h"
#include "util.h"

#define NUMREGS 16

#define CASE_CHIP8_KEY_RETURN(N) \
    case CHIP8_KEY_ ## N:        \
        return 0x ## N;

static CHIP8_ADDR g_mem[CHIP8_MEM_SZ] = {0};
static CHIP8_ADDR g_stack[CHIP8_STACK_SZ] = {0};
static uint8_t g_regv[NUMREGS] = {0};
static CHIP8_ADDR g_regi = 0;
static CHIP8_ADDR g_pc = 0;
static size_t g_sp = 0;
static uint16_t g_keystate = 0;

static uint8_t key_to_num(SDL_Keycode);
static void op_0(uint8_t);
static void op_8(uint8_t, uint8_t, uint8_t);
static void op_f(uint8_t, uint8_t);

void chip8_init(size_t scale) 
{
    scale = (scale) ? scale : SCREEN_DEFAULT_SCALE;
    chip8_reset();
    screen_init(scale);
}

void chip8_reset(void)
{
    for (size_t i = 0; i < CHIP8_MEM_SZ; ++i) {
        g_mem[i] = 0;
    }
    for (size_t i = 0; i < NUMREGS; ++i) {
        g_regv[i] = 0;
    }
    g_regi = 0;
    g_pc = 0;
    g_sp = 0;
    g_keystate = 0;
    screen_cls();
}

void chip8_destroy(void) 
{
    screen_destroy();
}

bool chip8_load(CHIP8_ADDR offset, CHIP8_ADDR img[], size_t num)
{
    if (offset + num >= CHIP8_MEM_SZ) {
        return false;
    }
    memcpy(g_mem + offset, img, num * sizeof(CHIP8_ADDR));
    return true;
}

void chip8_execute(CHIP8_ADDR entry)
{
    SDL_Event e = {0};
    for (g_pc = entry; g_pc < CHIP8_MEM_SZ - 1; ++g_pc) {
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == CHIP8_KEY_QUIT) {
                    return;
                }
                uint8_t keynum = key_to_num(e.key.keysym.sym);
                if (keynum < CHIP8_NUMKEYS) {
                    g_keystate |= (0x1 << keynum);
                }
            }
            else if (e.type == SDL_QUIT) {
                return;
            }
        }
        
        uint8_t op_hi = g_mem[g_pc] >> 8;
        uint8_t op_lo = g_mem[g_pc] & 0x00ff;
        CHIP8_ADDR addr_operand = g_mem[g_pc] & 0x0fff;
        size_t xreg = op_hi & 0x0f;
        size_t yreg = op_lo >> 4;

        switch (op_hi >> 4) {
            case 0x0: //CLS, RET
                op_0(op_lo);
                break;
            case 0x1: //JMP
                g_pc = addr_operand - 1;
                break;
            case 0x2: //CALL
                if (g_sp >= CHIP8_STACK_SZ) {
                    FAIL("g_stack overflow");
                }
                g_stack[g_sp] = g_pc;
                ++g_sp;
                g_pc = addr_operand;
                break;
            case 0x3: //JE
                if (g_regv[xreg] == op_lo) {
                    ++g_pc;
                }
                break;
            case 0x4: //JNE
                if (g_regv[xreg] != op_lo) {
                    ++g_pc;
                }
                break;
            case 0x5: //JRE
                if (g_regv[xreg] == g_regv[yreg]) {
                    ++g_pc;
                }
                break;
            case 0x6: //SETV
                g_regv[xreg] = op_lo;
                break;
            case 0x7: //ADD
                g_regv[xreg] += op_lo;
                break;
            case 0x8: //Math ops
                op_8(op_lo & 0x0f, xreg, yreg);
                break;
            case 0x9: //JRNE
                if (g_regv[xreg] != g_regv[yreg]) {
                    ++g_pc;
                }
                break;
            case 0xa: //SETI
                g_regi = addr_operand;
                break;
            case 0xb: //JR0
                g_pc = addr_operand + g_regv[0] - 1;
            case 0xc: //VRND
                g_regv[xreg] = op_lo & (rand() % 0xff);
                break;
            case 0xd://DRAW
                g_regv[0x0f] = screen_draw(
                    g_regv[xreg],
                    g_regv[yreg],
                    op_lo & 0x0f,
                    (uint8_t *)&g_mem[g_regi]
                );
                break;
            case 0xe: //Skip if [not] key
                {
                    CHIP8_ADDR xpressed = g_keystate & (0x1 << xreg);
                    switch (op_lo) {
                        case 0x9e:
                            if (xpressed) {
                                ++g_pc;
                                g_keystate = 0;
                            }
                            break;
                        case 0xa1:
                            if (!xpressed) {
                                ++g_pc;
                                g_keystate = 0;
                            }
                            break;
                        default:
                            FAIL("illegal instruction");
                            break;
                    }
                }
                break;
            case 0xf: //Misc ops
                op_f(op_lo, xreg);
                break;
            default:
                FAIL("illegal instruction");
                break;
        }
    }
}

static uint8_t key_to_num(SDL_Keycode key) {
    switch (key) {
        CASE_CHIP8_KEY_RETURN(0)
        CASE_CHIP8_KEY_RETURN(1)
        CASE_CHIP8_KEY_RETURN(2)
        CASE_CHIP8_KEY_RETURN(3)
        CASE_CHIP8_KEY_RETURN(4)
        CASE_CHIP8_KEY_RETURN(5)
        CASE_CHIP8_KEY_RETURN(6)
        CASE_CHIP8_KEY_RETURN(7)
        CASE_CHIP8_KEY_RETURN(8)
        CASE_CHIP8_KEY_RETURN(9)
        CASE_CHIP8_KEY_RETURN(A)
        CASE_CHIP8_KEY_RETURN(B)
        CASE_CHIP8_KEY_RETURN(C)
        CASE_CHIP8_KEY_RETURN(D)
        CASE_CHIP8_KEY_RETURN(E)
        CASE_CHIP8_KEY_RETURN(F)
    }
    return CHIP8_NUMKEYS;
}

static void op_0(uint8_t op) 
{
    switch (op) {
        case 0xe0: //CLS
            screen_cls();
            break;
        case 0xee: //RET
            if (g_sp >= 0) {
                FAIL("g_stack underflow");
            }
            g_pc = g_stack[g_sp];
            --g_sp;
            break;
        default:
            FAIL("illegal instruction");
            break;
    }
}

static void op_8(uint8_t op, uint8_t x, uint8_t y) 
{
    switch (op) {
        case 0x0: //ASGN
            g_regv[x] = g_regv[y];
            break;
        case 0x1: //OR
            g_regv[x] |= g_regv[y];
            break;
        case 0x2: //AND
            g_regv[x] &= g_regv[y];
            break;
        case 0x3: //XOR
            g_regv[x] ^= g_regv[y];
            break;
        case 0x4: //ADD
            {
                uint8_t res = g_regv[x] + g_regv[y];
                g_regv[0xf] = (res < g_regv[x] || res < g_regv[y]) ? 0x1 : 0x0;
                g_regv[x] = res;
            }
            break;
        case 0x5: //SUBY
            {
                uint8_t res = g_regv[x] - g_regv[y];
                g_regv[0xf] = (res > g_regv[x]) ? 0x1 : 0x0;
                g_regv[x] = res;
            }
            break;
        case 0x6: //SHR
            g_regv[0xf] = g_regv[x] & 0xfe;
            g_regv[x] >>= 1;
            break;
        case 0x7: //SUBX
            {
                uint8_t res = g_regv[y] - g_regv[x];
                g_regv[0xf] = (res > g_regv[y]) ? 0x1 : 0x0;
                g_regv[x] = res;
            }
            break;
        default:
            FAIL("illegal instruction");
            break;
    }
}

static void op_f(uint8_t op, uint8_t x) 
{
    switch (op) {
        case 0x07: //GDEL
            FAIL("unimplemented");
            break;
        case 0x0a: //GKEY
            {
                SDL_Event e = {0};
                while (SDL_WaitEvent(&e)) {
                    if (e.type == SDL_KEYUP) {
                        uint8_t keynum = key_to_num(e.key.keysym.sym);
                        if (keynum < CHIP8_NUMKEYS) {
                            g_regv[x] = keynum;
                            break;
                        }
                    }
                }
                break;
            }
        case 0x15: //SDEL
            FAIL("unimplemented");
            break;
        case 0x18: //SSND
            FAIL("unimplemented");
            break;
        case 0x1e:
            {
                CHIP8_ADDR res = g_regi + g_regv[x];
                g_regv[0xf] = (res < g_regi) ? 0x1 : 0x0;
                g_regi = res;
            }
            break;
        case 0x29: //CHRX
            FAIL("unimplemented");
            break;
        case 0x33: //BCD
            FAIL("unimplemented");
            break;
        case 0x55: //REGD
            FAIL("unimplemented");
            break;
        case 0x65: //REGL
            FAIL("unimplemented");
            break;
        default:
            FAIL("illegal instruction");
            break;
    }
}
