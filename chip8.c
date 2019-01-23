#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "screen.h"
#include "util.h"

#define NUMREGS 16
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
        return 0x ## N;

#define FONTSET_SZ 80
static const uint8_t fontset[FONTSET_SZ] = { 
  0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
  0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
  0x90, 0x90, 0xf0, 0x10, 0x10, // 4
  0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
  0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
  0xf0, 0x10, 0x20, 0x40, 0x40, // 7
  0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
  0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
  0xf0, 0x90, 0xf0, 0x90, 0x90, // A
  0xe0, 0x90, 0xe0, 0x90, 0xe0, // B
  0xf0, 0x80, 0x80, 0x80, 0xf0, // C
  0xe0, 0x90, 0x90, 0x90, 0xe0, // D
  0xf0, 0x80, 0xf0, 0x80, 0xf0, // E
  0xf0, 0x80, 0xf0, 0x80, 0x80  // F
};

static uint8_t g_mem[CHIP8_MEM_SZ + FONTSET_SZ] = {0};
static uint16_t g_stack[CHIP8_STACK_SZ] = {0};
static uint8_t g_v[NUMREGS] = {0};
static uint16_t g_i = 0;
static uint16_t g_pc = 0;
static size_t g_sp = 0;
static uint16_t g_keystate = 0;
static uint8_t g_delay = 0;

static uint8_t key_to_num(SDL_Keycode);
static void op_0(uint8_t);
static void op_8(uint8_t, uint8_t, uint8_t);
static void op_f(uint8_t, uint8_t);

void chip8_init(size_t scale) 
{
    chip8_reset();
    memcpy(&g_mem[CHIP8_MEM_SZ], fontset, FONTSET_SZ);
    scale = (scale) ? scale : SCREEN_DEFAULT_SCALE;
    screen_init(scale);
}

void chip8_reset(void)
{
    for (size_t i = 0; i < CHIP8_MEM_SZ; ++i) {
        g_mem[i] = 0;
    }
    for (size_t i = 0; i < NUMREGS; ++i) {
        g_v[i] = 0;
    }
    g_i = 0;
    g_pc = 0;
    g_sp = 0;
    g_keystate = 0;
    screen_cls();
}

void chip8_destroy(void) 
{
    screen_destroy();
}

bool chip8_load(uint16_t offset, uint8_t const img[], size_t num)
{
    if (offset + num >= CHIP8_MEM_SZ) {
        return false;
    }
    memcpy(g_mem + offset, img, num);
    return true;
}

void chip8_execute(uint16_t entry)
{
    SDL_Event e = {0};
    for (g_pc = entry; g_pc < CHIP8_MEM_SZ - 2; g_pc += 2) {
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYUP) {
                if (e.key.keysym.sym == KEY_QUIT) {
                    return;
                }
                uint8_t keynum = key_to_num(e.key.keysym.sym);
                if (keynum < NUMKEYS) {
                    g_keystate |= (0x1 << keynum);
                }
            }
            else if (e.type == SDL_QUIT) {
                return;
            }
        }
        
        uint8_t op_hi = g_mem[g_pc];
        uint8_t op_lo = g_mem[g_pc + 1];
        uint8_t addr_operand = op_lo + ((op_hi & 0x0f) << 8);
        size_t xreg = op_hi & 0x0f;
        size_t yreg = op_lo >> 4;

#ifdef TRACE
        printf(
            "%03x: (%02x %02x) x %lx, y %lx, addr %03x\n",
            g_pc, op_hi, op_lo, xreg, yreg, addr_operand
        );
#endif

        switch (op_hi >> 4) {
            case 0x0: //CLS, RET
                op_0(op_lo);
                break;
            case 0x1: //JMP
                g_pc = addr_operand - 2;
                break;
            case 0x2: //CALL
                if (g_sp >= CHIP8_STACK_SZ) {
                    FAIL("g_stack overflow");
                }
                g_stack[g_sp] = g_pc;
                ++g_sp;
                g_pc = addr_operand - 2;
                break;
            case 0x3: //JE
                if (g_v[xreg] == op_lo) {
                    g_pc += 2;
                }
                break;
            case 0x4: //JNE
                if (g_v[xreg] != op_lo) {
                    g_pc += 2;
                }
                break;
            case 0x5: //JRE
                if (g_v[xreg] == g_v[yreg]) {
                    g_pc += 2;
                }
                break;
            case 0x6: //SETV
                g_v[xreg] = op_lo;
                break;
            case 0x7: //ADD
                g_v[xreg] += op_lo;
                break;
            case 0x8: //Math ops
                op_8(op_lo & 0x0f, xreg, yreg);
                break;
            case 0x9: //JRNE
                if (g_v[xreg] != g_v[yreg]) {
                    g_pc += 2;
                }
                break;
            case 0xa: //SETI
                g_i = addr_operand;
                break;
            case 0xb: //JR0
                g_pc = addr_operand + g_v[0] - 2;
            case 0xc: //VRND
                g_v[xreg] = op_lo & (rand() % 0xff);
                break;
            case 0xd://DRAW
                g_v[0x0f] = screen_draw(
                    g_v[xreg],
                    g_v[yreg],
                    op_lo & 0x0f,
                    (uint8_t *)&g_mem[g_i]
                );
                break;
            case 0xe: //Skip if [not] key
                {
                    uint8_t xpressed = g_keystate & (0x1 << xreg);
                    switch (op_lo) {
                        case 0x9e:
                            if (xpressed) {
                                g_pc += 2;
                                g_keystate = 0;
                            }
                            break;
                        case 0xa1:
                            if (!xpressed) {
                                g_pc += 2;
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
        CASE_KEY_RETURN(0)
        CASE_KEY_RETURN(1)
        CASE_KEY_RETURN(2)
        CASE_KEY_RETURN(3)
        CASE_KEY_RETURN(4)
        CASE_KEY_RETURN(5)
        CASE_KEY_RETURN(6)
        CASE_KEY_RETURN(7)
        CASE_KEY_RETURN(8)
        CASE_KEY_RETURN(9)
        CASE_KEY_RETURN(A)
        CASE_KEY_RETURN(B)
        CASE_KEY_RETURN(C)
        CASE_KEY_RETURN(D)
        CASE_KEY_RETURN(E)
        CASE_KEY_RETURN(F)
    }
    return NUMKEYS;
}

static void op_0(uint8_t op) 
{
    switch (op) {
        case 0xe0: //CLS
            screen_cls();
            break;
        case 0xee: //RET
            if (g_sp <= 0) {
                FAIL("g_stack underflow");
            }
            --g_sp;
            g_pc = g_stack[g_sp];
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
            g_v[x] = g_v[y];
            break;
        case 0x1: //OR
            g_v[x] |= g_v[y];
            break;
        case 0x2: //AND
            g_v[x] &= g_v[y];
            break;
        case 0x3: //XOR
            g_v[x] ^= g_v[y];
            break;
        case 0x4: //ADD
            {
                uint8_t res = g_v[x] + g_v[y];
                g_v[0xf] = (res < g_v[x] || res < g_v[y]) ? 0x1 : 0x0;
                g_v[x] = res;
            }
            break;
        case 0x5: //SUBY
            {
                uint8_t res = g_v[x] - g_v[y];
                g_v[0xf] = (res > g_v[x]) ? 0x1 : 0x0;
                g_v[x] = res;
            }
            break;
        case 0x6: //SHR
            g_v[0xf] = g_v[x] & 0xfe;
            g_v[x] >>= 1;
            break;
        case 0x7: //SUBX
            {
                uint8_t res = g_v[y] - g_v[x];
                g_v[0xf] = (res > g_v[y]) ? 0x1 : 0x0;
                g_v[x] = res;
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
                        if (e.key.keysym.sym == KEY_QUIT) {
                            exit(EXIT_SUCCESS);
                        } else if (keynum < NUMKEYS) {
                            g_v[x] = keynum;
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
                uint8_t res = g_i + g_v[x];
                g_v[0xf] = (res < g_i) ? 0x1 : 0x0;
                g_i = res;
            }
            break;
        case 0x29: //CHRX
            g_i = CHIP8_MEM_SZ + g_v[x]*5;
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
