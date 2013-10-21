#include <stdio.h>
#include <stdlib.h>
#include "cpu/cpu.h"

struct CPU_State currentState; 

int main(int argc, char *argv[]){
    CPU_Init(&currentState);
    if(argc >= 2) {
        CPU_LoadImageFromFile(&currentState, argv[1], PROGRAM_START);
    }
    char choice = 's';
    while(CPU_IsRunning(&currentState)){
        CPU_Step(&currentState);
        if(choice != 'r'){
            CPU_DebugStatus(&currentState);
            printf("\n(r)un/(s)tep/(d)ump memory/(q)uit?");
            scanf(" %c", &choice);
            printf("\n");
            if(choice == 'd') CPU_DumpMemoryToFile(&currentState, "memdump.dmp");
            if(choice == 'q') break;
        }
    }
    return 0;
}
