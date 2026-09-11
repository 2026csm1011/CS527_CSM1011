#include <stdio.h>
#include <stdint.h>
#include "memory.h"

unsigned char Instruction[NP][256] = {0};
unsigned char Data[NP][4096] = {0}; 

void load_memory(int proc_id, const char *prog_file, const char *data_file) {
    if (proc_id < 0 || proc_id >= NP) return;

    // Load Instructions into the specific core
    FILE *prog = fopen(prog_file, "r");
    if (prog) 
    {
        int op, d, s1, s2, i = 0;
        while (fscanf(prog, "%d %d %d %d", &op, &d, &s1, &s2) == 4) 
        {
            if (i >= 252) break; 
            Instruction[proc_id][i++] = (unsigned char)op;
            Instruction[proc_id][i++] = (unsigned char)d;
            Instruction[proc_id][i++] = (unsigned char)s1;
            Instruction[proc_id][i++] = (unsigned char)s2;
        }
        fclose(prog);
    }
    
    // Load Data into the specific core
    FILE *data = fopen(data_file, "r");
    if (data) 
    {
        int op, d, s1, s2, i = 0;
        while (fscanf(data, "%i %i %i %i", &op, &d, &s1, &s2) == 4) 
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
        for (int i = 0; i < 4096; i += 4) 
        {
            fprintf(data, "%d %d %d %d\n", 
                Data[proc_id][i], Data[proc_id][i+1], Data[proc_id][i+2], Data[proc_id][i+3]);
        }
        fclose(data);
    }
}