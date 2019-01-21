#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "screen.h"
#include "util.h"

#define INSTR_SZ 2
#define NUMREGS 16

static uint8_t mem[CHIP8_MEM_SZ] = {0};
static uint16_t stack[CHIP8_STACK_SZ] = {0};
static uint8_t reg_v[NUMREGS] = {0};
static uint16_t reg_i = 0;
static uint16_t pc = 0;
static size_t sp = 0;

static void key(uint8_t, uint8_t);
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
        mem[i] = 0;
    }
    for (size_t i = 0; i < NUMREGS; ++i) {
        reg_v[i] = 0;
    }
    reg_i = 0;
    pc = 0;
    sp = 0;
    screen_cls();
}

void chip8_destroy(void) 
{
    screen_destroy();
}

bool chip8_load(uint16_t offset, uint8_t bytes[], size_t num)
{
    if (offset + num >= CHIP8_MEM_SZ) {
        return false;
    }
    memcpy(mem + offset, bytes, num);
    return true;
}

void chip8_execute(uint16_t entry)
{
    SDL_Event e = {0};
    for (pc = entry; pc < CHIP8_MEM_SZ; pc += INSTR_SZ) {
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
                if (sp >= CHIP8_STACK_SZ) {
                    FAIL("stack overflow");
                }
                stack[sp] = pc;
                ++sp;
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
                reg_v[0x0f] = screen_draw(
                    reg_v[reg_operand_x],
                    reg_v[reg_operand_y],
                    op_lo & 0x0f,
                    &mem[reg_i]
                );
                break;
            case 0xe: //Skip if [not] key
                key(op_lo, reg_operand_x);
                break;
            case 0xf: //Misc ops
                op_f(op_lo, reg_operand_x);
                break;
            default:
                FAIL("illegal instruction");
                break;
        }
    }
}

static inline void key(uint8_t op, uint8_t reg) {}

static void op_0(uint8_t op) 
{
    switch (op) {
        case 0xe0: //CLS
            screen_cls();
            break;
        case 0xee: //RET
            pc = stack[sp];
            --sp;
            if (sp > CHIP8_STACK_SZ) {
                FAIL("stack underflow");
            }
            break;
        default:
            FAIL("illegal instruction");
    }
}

static void op_8(uint8_t op, uint8_t x, uint8_t y) 
{
    switch (op) {
        case 0x0: //ASGN
            reg_v[x] = reg_v[y];
            break;
        case 0x1: //OR
            reg_v[x] |= reg_v[y];
            break;
        case 0x2: //AND
            reg_v[x] &= reg_v[y];
            break;
        case 0x3: //XOR
            reg_v[x] ^= reg_v[y];
            break;
        case 0x4: //ADD
            {
                uint8_t res = reg_v[x] + reg_v[y];
                if (res < reg_v[x] || res < reg_v[y]) {
                    reg_v[0xf] = 0x1;
                }
                reg_v[x] = res;
            }
            break;
        case 0x5: //SUBY
            {
                uint8_t res = reg_v[x] - reg_v[y];
                if (res > reg_v[x]) {
                    reg_v[0xf] = 0x1;
                }
                reg_v[x] = res;
            }
            break;
        case 0x6: //SHR
            reg_v[0xf] = reg_v[x] & 0xfe;
            reg_v[x] >>= 1;
            break;
        case 0x7: //SUBX
            {
                uint8_t res = reg_v[y] - reg_v[x];
                if (res > reg_v[y]) {
                    reg_v[0xf] = 0x1;
                }
                reg_v[x] = res;
            }
            break;
        default:
            FAIL("illegal instruction");
    }
}

static void op_f(uint8_t op, uint8_t x) 
{
    FAIL("unimplemented");
}