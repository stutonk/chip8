/*
    Table of Chip8 Opcodes
    00e0    CLS                 Clear the screen
    00ee    RET                 Return from subroutine
    1nnn    JP      nnn         goto nnn
    2nnn    CALL    nnn         Call subroutine at addr nnn
    3xnn    SE      vx  nn      Skip next instr if vx == nn
    4xnn    SNE     vx  nn      Skip next instr if vx != nn
    5xy0    SRE     vx  vy      Skip next instr if vx == vy
    6xnn    LD      vx  nn      vx = nn
    7xnn    ADD     vx  nn      vx += nn
    8xy0    MV      vx  vy      vx = vy
    8xy1    OR      vx  vy      vx = vx | vy
    8xy2    AND     vx  vy      vx = vx & vy
    8xy3    XOR     vx  vy      vx = vx ^ vy
    8xy4    ADDR    vx  vy  vf* vx += vy; vf = (carry) ? 1 : 0
    8xy5    SUBY    vx  vy  vf* vx -= vy; vf = (carry) ? 1 : 0
    8x_6    SHR     vx  vf*     vx >>= 1; vf = vx & 0x01
    8xy7    SUBX    vx  vy  vf* vx = yv - vx; vf = (carry) ? 1 : 0
    8x_e    SHL     vx  vf*     vx <<= 1; vf = (vx & 0x80) >> 7
    9xy0    SRNE    vx  vy      Skip next inst if vx != vy
    annn    LDI     nnn i*      i = nnn
    bnnn    JMPI    nnn         pc = nnn + v0
    cxnn    RAND    vx  nn      vx = rand() & nn
    dxyn    DRAW    vx  vy  n   Draw sprite starting addr i at x, y with
                                width 8 and height n
    ex93    SKP     vx          Skip next instr if key in vx was pressed
    exa1    SKNP    vx          Skip next instr if key in vs wasn't pressed
    fx07    MVD     vx          Set vx to value of delay timer
    fx0a    KEY     vx          vx = Halt and wait for keypress
    fx15    LDD     vx          Set delay timer to value in vx
    fx18    LDS     vx          Set sound timer to value in vx
    fx1e    ADDI    vx  i*      i += vx
    fx29    LDSP    vx          Set i to the addr of the sprite corresponding
                                to the character in vx
    fx33    BCD     vx          Stores binary-coded decimal representation of
                                the number in vx in three consecutive addrs
                                starting at addr i; big-endian
    fx55    STOR    x   i*      Dump registers 0-x inclusive into memory
                                starting at addr i
    fx65    READ    x   i*      Load registers 0-x inclusive from memory
                                starting at addr i
    * implicit operand
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chip8.h"
#include "input.h"
#include "screen.h"
#include "timer.h"
#include "util.h"

#define NUMREGS 16

/*
    The Chip8 standard fontset contains sprites corresponding to each
    key on the Chip8 keypad. In order to allow compatibility with Chip8
    opcodes as well as free up as much memory as possible, the fontset
    is loaded into an address range beyond that which is normally
    available to programs.
*/
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

static uint8_t Mem[CHIP8_MEM_SZ + FONTSET_SZ] = {0};
static uint16_t Stack[CHIP8_STACK_SZ] = {0};
static uint8_t V[NUMREGS] = {0};
static uint16_t I = 0;
static uint16_t Pc = 0;
static size_t Sp = 0;

void chip8_init(size_t scale) 
{
    chip8_reset();
    memcpy(&Mem[CHIP8_MEM_SZ], fontset, FONTSET_SZ);
    scale = (scale) ? scale : SCREEN_DEFAULT_SCALE;
    screen_init(scale);
    timer_init();
}

void chip8_reset(void)
{
    for (size_t i = 0; i < CHIP8_MEM_SZ; ++i) {
        Mem[i] = 0;
    }
    for (size_t i = 0; i < NUMREGS; ++i) {
        V[i] = 0;
    }
    I = 0;
    Pc = 0;
    Sp = 0;
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
    memcpy(Mem + offset, img, num);
    return true;
}

void chip8_execute(uint16_t entry)
{
    for (Pc = entry; Pc < CHIP8_MEM_SZ - 2; Pc += 2) {
        timer_update();
        input_update();
        
        uint8_t op_hi = Mem[Pc];
        uint8_t op_lo = Mem[Pc + 1];
        uint16_t op_addr = ((op_hi & 0x0f) << 8) + op_lo;
        size_t op_x = op_hi & 0x0f;
        size_t op_y = op_lo >> 4;

#ifdef TRACE
        printf(
            "%03x: (%02x %02x) -- %03x -- [%02x %02x %02x %02x %02x %02x "
                "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x] "
                "delay %d\n",
            Pc, op_hi, op_lo, I, V[0], V[1], V[2], V[3], V[4],
            V[5], V[6], V[7], V[8], V[9], V[10], V[11], V[12],
            V[13], V[14], V[15], Delay
        );
#endif
        switch (op_hi >> 4) {
            case 0x0:
                switch (op_lo) {
                    case 0xe0: //CLS
                        screen_cls();
                        break;
                    case 0xee: //RET
                        if (Sp <= 0) {
                            FAIL("stack underflow");
                        }
                        --Sp;
                        Pc = Stack[Sp];
                        break;
                    default:
                        goto unrecognized;
                        break;
                }
                break;
            case 0x1: //JP
                Pc = op_addr - 2;
                break;
            case 0x2: //CALL
                if (Sp >= CHIP8_STACK_SZ) {
                    FAIL("stack overflow");
                }
                Stack[Sp] = Pc;
                ++Sp;
                Pc = op_addr - 2;
                break;
            case 0x3: //SE
                if (V[op_x] == op_lo) {
                    Pc += 2;
                }
                break;
            case 0x4: //SNE
                if (V[op_x] != op_lo) {
                    Pc += 2;
                }
                break;
            case 0x5: //SRNE
                if (V[op_x] == V[op_y]) {
                    Pc += 2;
                }
                break;
            case 0x6: //LD
                V[op_x] = op_lo;
                break;
            case 0x7: //ADD
                V[op_x] += op_lo;
                break;
            case 0x8:
                switch (op_lo & 0x0f) {
                    case 0x0: //RCPY
                        V[op_x] = V[op_y];
                        break;
                    case 0x1: //OR
                        V[op_x] |= V[op_y];
                        break;
                    case 0x2: //AND
                        V[op_x] &= V[op_y];
                        break;
                    case 0x3: //XOR
                        V[op_x] ^= V[op_y];
                        break;
                    case 0x4: //ADDR
                        V[0xf] = ((int)V[op_x] + (int)V[op_y] > 0xff) ? 
                            0x1 : 0x0;
                        V[op_x] = V[op_x] + V[op_y];
                        break;
                    case 0x5: //SUBY
                        V[0xf] = (V[op_x] > V[op_y]) ? 0x1 : 0x0;
                        V[op_x] = V[op_x] - V[op_y];
                        break;
                    case 0x6: //SHR
                        V[0xf] = V[op_x] & 0xfe;
                        V[op_x] >>= 1;
                        break;
                    case 0x7: //SUBX
                        V[0xf] = (V[op_y] > V[op_x]) ? 0x1 : 0x0;
                        V[op_x] = V[op_y] - V[op_x];
                        break;
                    case 0xe: //SHL
                        V[0xf] = (V[op_x] & 0x80) >> 7;
                        V[op_x] <<= 1;
                        break;
                    default:
                        goto unrecognized;
                        break;
                }
                break;
            case 0x9: //SRNE
                if (V[op_x] != V[op_y]) {
                    Pc += 2;
                }
                break;
            case 0xa: //LDI
                I = op_addr;
                break;
            case 0xb: //JMPI
                Pc = op_addr + V[0] - 2;
            case 0xc: //RAND
                V[op_x] = op_lo & (rand() % 0xff);
                break;
            case 0xd://DRAW
                V[0x0f] = screen_draw(
                    V[op_x],
                    V[op_y],
                    op_lo & 0x0f,
                    &Mem[I]
                );
                break;
            case 0xe:
                switch (op_lo) {
                    case 0x9e: //SKP
                        Pc += (input_query(V[op_x])) ? 2 : 0;
                        break;
                    case 0xa1: //SKNP
                        Pc += (input_query(V[op_x])) ? 0 : 2;
                        break;
                    default:
                        goto unrecognized;
                }
                break;
            case 0xf:
                switch (op_lo) {
                    case 0x07: //MVD
                        V[op_x] = timer_get_delay();
                        break;
                    case 0x0a: //KEY
                        V[op_x] = input_get_key();
                        break;
                    case 0x15: //LDD
                        timer_set_delay(V[op_x]);
                        break;
                    case 0x18: //LDS
                        timer_set_sound(V[op_x]);
                        break;
                    case 0x1e: //ADDI
                        V[0xf] = (I + V[op_x] > 0xfff) ? 0x1 : 0x0;
                        I += V[op_x];
                        break;
                    case 0x29: //LDSP
                        I = CHIP8_MEM_SZ + V[op_x]*5;
                        break;
                    case 0x33: //BCD
                        if (I > CHIP8_MEM_SZ - 3) {
                            FAIL("BCD causes memory overflow");
                        }
                        {
                            uint8_t res = V[op_x];
                            Mem[I] = res / 100;
                            res %= 100;
                            Mem[I + 1] = res / 10;
                            Mem[I + 2] = res % 10;
                        }
                            break;
                    case 0x55: //STOR
                        if (I + op_x + 1 >= CHIP8_MEM_SZ) {
                            FAIL("REGD causes memory overflow");
                        }
                        for (size_t i = 0; i <= op_x; ++i) {
                            Mem[I + i] = V[i];
                        }
                        break;
                    case 0x65: //READ
                        if (I + op_x + 1 >= CHIP8_MEM_SZ) {
                            FAIL("REGL accesses illegal address");
                        }
                        for (size_t i = 0; i <= op_x; ++i) {
                            V[i] = Mem[I + i];
                        }
                        break;
                    default:
                        goto unrecognized;
                        break;
                }
                break;
            default:
                goto unrecognized;
                break;
        }
        continue;
unrecognized:
        printf("%s: unrecognized opcode: %02x%02x\n", __func__, op_hi, op_lo);
        chip8_destroy();
        exit(EXIT_FAILURE);
    }
}
