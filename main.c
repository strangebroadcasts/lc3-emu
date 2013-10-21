#include <stdio.h>
#include <stdlib.h>
#include "cpu/cpu.h"

struct CPU_State currentState;

int main(int argc, char *argv[]){
    CPU_Init(&currentState);
    while(CPU_IsRunning(&currentState)){
        CPU_Step(&currentState);
        CPU_DebugStatus(&currentState);
        getchar();
    }
    CPU_DumpMemoryToFile(&currentState, "memdump.dmp");
    return 0;
}
