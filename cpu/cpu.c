#include "cpu.h"

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
    state->pc = 0x3000;
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
        state->reg[7] = state->pc;
        state->pc = state->mem[TRAPVECT8(inst)] - 1;
        break;

    default: // ILLEGAL OPCODE - shouldn't actually happen
        break;

    }
    state->pc++;
}
