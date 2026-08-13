#include <stdio.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main() {
    printf("[System] Starting compilation...\n");
    compile();
    
    printf("[System] Loading memory modules...\n");
    initialize();
    
    printf("[System] Booting CPU...\n");
    reset();
    
    printf("[System] Executing instructions...\n");
    while(end_of_simulation == 0) {
        fetch();
        decode();
        execute();
    }
    
    printf("[System] Execution finished. Saving data.byte...\n");
    finalize();
    
    printf("\n--- FINAL REGISTER STATES ---\n");
    for (int i = 0; i < 256; i++) {
        if (Register[i] != 0) {
            printf("Reg[%d] = %d\n", i, Register[i]);
        }
    }
    printf("-----------------------------\n");
    
    return 0;
}