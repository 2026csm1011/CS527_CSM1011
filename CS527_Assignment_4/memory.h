#ifndef MEMORY_H
#define MEMORY_H

#define NP 4 // Number of Processors

extern unsigned char Instruction[NP][256];
extern unsigned char Data[NP][4096];

// Updated to load and save for a specific core
void load_memory(int proc_id, const char *prog_file, const char *data_file);
void save_memory(int proc_id, const char *data_file);

#endif