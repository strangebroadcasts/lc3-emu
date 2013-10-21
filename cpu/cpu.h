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

// device registers
#define KBSR 0xFE00
#define KBDR 0xFE02
#define DSR 0xFE04
#define DDR 0xFE06
#define MCR 0xFFFE

#define PROGRAM_START 0x3000

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
    int running;
};

void CPU_Init(struct CPU_State *state);
void CPU_Step(struct CPU_State *state);
void CPU_LoadImageFromFile(struct CPU_State *state, char *filename, int offset);
void CPU_DumpMemoryToFile(struct CPU_State *state, char *filename);
void CPU_DebugStatus(struct CPU_State *state);
void CPU_HandleTrap(struct CPU_State *state, int trap);

int CPU_IsRunning(struct CPU_State *state);

#endif /*CPU_H_INCLUDED*/
