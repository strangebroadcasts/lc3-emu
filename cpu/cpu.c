#include <stdio.h>
#include "cpu.h"

#define PROGRAM_START 0x3000

// Let the LC3 OS handle system calls?
#define NATIVE_TRAP_HANDLING 0

// TODO:
// * proper interrupt implementation
// * privilege mode exception

// sign extension:
// extend the number x stored in
// b bits to a 16 bit int

int16_t SEXT(x,b){
    int m = 1U << (b - 1);
    x = x & ((1U << b) - 1);
    return (x^m)-m;
}

// set condition codes
void CPU_SetCC(struct CPU_State *state) {

    int result = state->reg[DR(state->mem[state->pc])];

    // clear flags
    state->psr.n = 0;
    state->psr.z = 0;
    state->psr.p = 0;

    if(result > 0) {
        state->psr.p = 1;
    } else if (result == 0) {
        state->psr.z = 1;
    } else {
        state->psr.n = 1;
    }
}

void CPU_Init(struct CPU_State *state)
{
    state->psr.privmode = 0;
    state->psr.priority = 0;
    state->psr.n = 0;
    state->psr.z = 0;
    state->psr.p = 0;
    state->pc = PROGRAM_START;
    state->mem[MCR] = (1 << 15);
}

void CPU_Step(struct CPU_State *state)
{
    uint16_t inst = state->mem[state->pc];

    switch(OPCODE(inst)){
    case 0b0000: // BRANCH
        if((BIT_SET(inst, 11) && state->psr.n) ||
           (BIT_SET(inst, 10) && state->psr.z) ||
           (BIT_SET(inst, 9) && state->psr.p)){
            state->pc += SEXT(PCOFFSET9(inst), 9);
        }
        break;

    case 0b0001: // ADD
        if(BIT_SET(inst, 5)) {
            state->reg[DR(inst)] = state->reg[SR1(inst)] + SEXT(IMM5(inst), 5);
        } else {
            state->reg[DR(inst)] = state->reg[SR1(inst)] + state->reg[SR2(inst)];
        }
        CPU_SetCC(state);
        break;

    case 0b0010: // LOAD
        state->reg[DR(inst)] = state->mem[state->pc + 1 + SEXT(PCOFFSET9(inst), 9)];
        CPU_SetCC(state);
        break;

    case 0b0011: // STORE
        state->mem[state->pc + 1 + SEXT(PCOFFSET9(inst), 9)] = state->reg[DR(inst)];
        break;

    case 0b0100: // JUMP TO SUBROUTINE
        state->reg[7] = state->pc + 1;

        if(BIT_SET(inst, 11)) {
            state->pc += SEXT(PCOFFSET11(inst), 11);
        } else {
            state->pc = state->reg[SR1(inst)] - 1;
        }
        break;

    case 0b0101: // BITWISE AND
        if(BIT_SET(inst, 5)) {
            state->reg[DR(inst)] = state->reg[SR1(inst)] & state->reg[SR2(inst)];
        } else {
            state->reg[DR(inst)] = state->reg[SR1(inst)] & SEXT(IMM5(inst), 5);
        }
        CPU_SetCC(state);
        break;

    case 0b0110: // LOAD BASE + OFFSET
        state->reg[DR(inst)] = state->mem[state->reg[
            SR1(inst)] + SEXT(OFFSET6(inst), 6)];
        CPU_SetCC(state);
        break;

    case 0b0111: // STORE BASE + OFFSET
        state->mem[state->reg[DR(inst)] + SEXT(OFFSET6(inst), 6)] =
            state->reg[SR1(inst)];
        break;

    case 0b1000: // RETURN FROM INTERRUPT
        if(BIT_SET(inst,15)){
            // initiate privilege mode exception
        } else {
            state->pc = state->mem[state->reg[6]];
            state->reg[6]++;
        }
        break;

    case 0b1001: // BITWISE COMPLEMENT
        state->reg[DR(inst)] = ~state->reg[SR1(inst)];
        CPU_SetCC(state);
        break;

    case 0b1010: // LOAD INDIRECT
        state->reg[DR(inst)] = state->mem[state->mem[
            state->pc + 1 + SEXT(PCOFFSET9(inst), 9)]];
        CPU_SetCC(state); 
        break;

    case 0b1011: // STORE INDIRECT
        state->mem[state->mem[state->pc + 1 + SEXT(PCOFFSET9(inst),9)]] =
            state->reg[DR(inst)];
        break;

    case 0b1100: // JMP/RET
        state->pc = state->reg[SR1(inst)] - 1;
        break;

    case 0b1101: // RESERVED/ILLEGAL OPCODE
        break;

    case 0b1110: // LOAD EFFECTIVE ADDRESS
        state->reg[DR(inst)] = state->pc + 1 + SEXT(PCOFFSET9(inst), 9);
        CPU_SetCC(state);
        break;

    case 0b1111: // TRAP
        state->reg[7] = state->pc + 1;
        if(NATIVE_TRAP_HANDLING){
            state->pc = state->mem[TRAPVECT8(inst)] - 1;
        } else {
            CPU_HandleTrap(state, TRAPVECT8(inst));
        }
        break;

    default: // ILLEGAL OPCODE - shouldn't actually happen
        break;

    }
    state->pc++;
}

// Handle system calls by routing them to standard I/O functions.
// If the OS should handle the calls instead,
// set NATIVE_TRAP_HANDLING to 1.
void CPU_HandleTrap(struct CPU_State *state, int trap)
{
    int offset;
    switch(trap){
    case 0x20: // GETC - read character into R0 without prompt
        state->reg[0] = getchar();
        break;
    case 0x21: // OUT - write character in R0 to display
        printf("%c", state->reg[0] & 0xff);
        break;
    case 0x22: // PUTS - write string to screen.
        offset = state->reg[0];
        while(state->mem[offset] != 0 && offset < 0x10000){
            printf("%c", state->mem[offset] & 0xff);
            offset++;
        }
        break;
    case 0x23: // IN - read single character into R0 with prompt
        printf("?");
        char console_input;
        scanf("%c", &console_input);
        state->reg[0] = console_input;
        break;
    case 0x24: // PUTSP - write string (using two characters for each memory location) 
        offset = state->reg[0];
        while(state->mem[offset] != 0 && offset < 0x10000){
            printf("%c%c", state->mem[offset] & 0xff, (state->mem[offset] >> 8) & 0xff);
            offset++;
        }
        break;
    case 0x25: // HALT
        state->mem[MCR] = 0; // set the MCR to 0, stopping the computer
        break;
    default: // illegal system call
        fprintf(stderr, "ERROR: called unknown trap routine %x", trap); 
        break;
    }
    state->pc = state->reg[7];
}

void CPU_LoadImageFromFile(struct CPU_State *state, char *filename, int offset)
{
    FILE *image = NULL;
    image = fopen(filename, "rb");
    fread(&state->mem[offset], sizeof(int16_t), 0x10000 - offset, image);
    fclose(image);
}

void CPU_DumpMemoryToFile(struct CPU_State *state, char *filename)
{
    FILE *memdump = NULL;
    memdump = fopen(filename, "wb");
    fwrite(&state->mem, sizeof(int16_t), 0x10000, memdump);
    fclose(memdump);
}

void CPU_DebugStatus(struct CPU_State *state)
{
    printf("Currently executing instruction %x\n", state->pc);
    printf("Registers: ");
    int i = 0;
    for(i = 0; i < 8; i++) {
        printf("R%d = %d, ", i, state->reg[i]);
    }

    printf("\nPSR: Running in ");
    if(state->psr.privmode == 0){
        printf("supervisor");
    } else {
        printf("user");
    }
    printf(" mode, priority level %d, flags: N = %d, Z = %d, P = %d\n",
        state->psr.priority, state->psr.n, state->psr.z, state->psr.p);
}

int CPU_IsRunning(struct CPU_State *state){
    return state->mem[MCR];
}
