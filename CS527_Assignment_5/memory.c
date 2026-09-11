#include <stdio.h>
#include <stdint.h>
#include "memory.h"

// The single physical memory for all processors
char memory[MEMSIZE] = {0};

// Logical memory buffers
char Instruction[NP][256] = {0};
char Data[NP][4096] = {0}; 

void load_memory(int proc_id, const char *prog_file, const char *data_file) {
    if (proc_id < 0 || proc_id >= NP) return;

    // Load Instructions into the logical buffer
    FILE *prog = fopen(prog_file, "r");
    if (prog) 
    {
        int op, d, s1, s2, i = 0;
        while (fscanf(prog, "%x %x %x %x", &op, &d, &s1, &s2) == 4) 
        {
            if (i >= 252) break; 
            Instruction[proc_id][i++] = (unsigned char)op;
            Instruction[proc_id][i++] = (unsigned char)d;
            Instruction[proc_id][i++] = (unsigned char)s1;
            Instruction[proc_id][i++] = (unsigned char)s2;
        }
        fclose(prog);
    }
    
    // Load Data into the logical buffer
    FILE *data = fopen(data_file, "r");
    if (data) 
    {
        int op, d, s1, s2, i = 0;
        while (fscanf(data, "%x %x %x %x", &op, &d, &s1, &s2) == 4) 
        {
            if (i >= 4092) break;
            Data[proc_id][i++] = (unsigned char)op;
            Data[proc_id][i++] = (unsigned char)d;
            Data[proc_id][i++] = (unsigned char)s1;
            Data[proc_id][i++] = (unsigned char)s2;
        }
        fclose(data);
    }
}

void save_memory(int proc_id, const char *data_file) 
{
    if (proc_id < 0 || proc_id >= NP) return;

    FILE *data = fopen(data_file, "w");
    if (data) 
    {
        int last_used = 0;
        // Find the last non-zero byte to avoid printing 1000 lines of empty zeroes
        for (int i = 4095; i >= 0; i--) {
            if (Data[proc_id][i] != 0) {
                last_used = i;
                break;
            }
        }
        
        // Guarantee we at least print up to address 256 so we don't miss anything
        if (last_used < 256) last_used = 256;

        for (int i = 0; i <= last_used; i += 4) 
        {
            fprintf(data, "%02X %02X %02X %02X\n", 
                (unsigned char)Data[proc_id][i], 
                (unsigned char)Data[proc_id][i+1], 
                (unsigned char)Data[proc_id][i+2], 
                (unsigned char)Data[proc_id][i+3]);
        }
        fclose(data);
    }
}