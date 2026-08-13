#include <stdio.h>
#include "memory.h"

char Instruction[256];
char Data[256];

void initialize() {
    // Open program.byte using the absolute path to load instructions into memory
    FILE *prog_file = fopen("C:\\Users\\Indrasen Gupta\\Desktop\\CS527_Assignment_1\\program.byte", "rb");
    if (prog_file) {
        fread(Instruction, 1, 256, prog_file);
        fclose(prog_file);
    } else {
        printf("Error: Could not read program.byte for initialization.\n");
    }
    
    // Open data.byte using the absolute path to load existing data into memory
    FILE *data_file = fopen("C:\\Users\\Indrasen Gupta\\Desktop\\CS527_Assignment_1\\data.byte", "rb");
    if (data_file) {
        fread(Data, 1, 256, data_file);
        fclose(data_file);
    }
}

void finalize() {
    // Save the final state of the data memory back to data.byte using the absolute path
    FILE *data_file = fopen("C:\\Users\\Indrasen Gupta\\Desktop\\CS527_Assignment_1\\data.byte", "wb");
    if (data_file) {
        fwrite(Data, 1, 256, data_file);
        fclose(data_file);
    } else {
        printf("Error: Could not save to data.byte during finalization.\n");
    }
}