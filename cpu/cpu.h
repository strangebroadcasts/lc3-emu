#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <stdint.h>

// bit twiddling macros
#define BIT_SET(x,y) ((x >> y) & 0x1)
#define OPCODE(x) (x >> 12)
#define DR(x) ((x >> 8) & 0x7)
#define SR1(x) ((x >> 5) & 0x7)
#define SR2(x) (x & 0x7)
#define IMM5(x) (x & 0x1f)
#define OFFSET6(x) (x & 0x3f)
#define PCOFFSET9(x) (x & 0x1ff)
#define PCOFFSET11(x) (x & 0x3ff)
#define TRAPVECT8(x) (x & 0xff)

struct CPU_StatusRegister {
    unsigned int privmode: 1;
    unsigned int : 4;
    unsigned int priority: 3;
    unsigned int : 5;
    unsigned int n : 1;
    unsigned int z : 1;
    unsigned int p : 1;
};

struct CPU_State {
    uint16_t mem[0x10000];
    int16_t reg[8];
    uint16_t pc;
    struct CPU_StatusRegister psr;
};

void CPU_Init(struct CPU_State *state);
void CPU_Step(struct CPU_State *state);

#endif /*CPU_H_INCLUDED*/
