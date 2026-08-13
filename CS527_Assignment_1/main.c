#include <stdio.h>
#include "compiler.h"
#include "memory.h"
#include "processor.h"

int main() {
    // Compile the selected input file into program.byte
    compile();
    
    // Initialize system memory and reset processor state
    initialize();
    reset();
    
    // Core CPU execution loop
    while(end_of_simulation == 0) {
        fetch();
        decode();
        execute();
    }
    
    // Finalize simulation and clear resources
    finalize();
    printf("\nSimulation complete. Check data.byte for memory outputs.\n");
    
    // Output final register states to the console
    printf("\n--- FINAL REGISTER STATES ---\n");
    for (int i = 0; i < 256; i++) {
        if (Register[i] != 0) {
            printf("Reg[%d] = %d\n", i, Register[i]);
        }
    }
    printf("-----------------------------\n");
    
    // Append execution status and final results directly to program.byte
    FILE *byte_file = fopen("C:\\Users\\Indrasen Gupta\\Desktop\\CS527_Assignment_1\\program.byte", "a"); 
    
    if (byte_file != NULL) {
        fprintf(byte_file, "\n====================================\n");
        fprintf(byte_file, "STATUS: Execution Successful\n");
        fprintf(byte_file, "====================================\n");
        fprintf(byte_file, "FINAL ANSWERS (Non-Zero Registers):\n");
        
        for (int i = 0; i < 256; i++) {
            if (Register[i] != 0) {
                fprintf(byte_file, "-> Register[%d] holds value: %d\n", i, Register[i]);
            }
        }
        fprintf(byte_file, "====================================\n");
        
        fclose(byte_file);
    } else {
        printf("\nError: Could not open program.byte for appending.\n");
    }
    
    return 0;
}