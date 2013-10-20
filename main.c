#include <stdio.h>
#include <stdlib.h>
#include "cpu/cpu.h"

struct CPU_State currentState;

int main(int argc, char *argv[]){
    CPU_Init(&currentState);
    CPU_Step(&currentState);
    return 0;
}
