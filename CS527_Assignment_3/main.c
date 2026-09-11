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
    
    printf("\n--- FINAL SCALAR REGISTER STATES ---\n");
    for (int i = 0; i < 256; i++) {
        if (Register[i] != 0) {
            printf("Reg[%d] = %d\n", i, Register[i]);
        }
    }
    
    printf("\n--- FINAL VECTOR REGISTER STATES ---\n");
    for (int i = 0; i < 8; i++) {
        int has_data = 0;
        // Check if the vector register has any non-zero data
        for(int j = 0; j < 8; j++) {
            if(VReg[i][j] != 0) has_data = 1;
        }
        // If it was used, print all 8 elements!
        if (has_data) {
            printf("VReg[%d] = [ ", i);
            for (int j = 0; j < 8; j++) {
                printf("%d ", VReg[i][j]);
            }
            printf("]\n");
        }
    }
    printf("-----------------------------\n\n");
    
    return 0;
}