#ifndef MEMORY_H
#define MEMORY_H

// Lab 5 Defines
#define NP 4 // Number of processors
#define MAX_PROC 4
#define MEMSIZE 8192
#define PAGESIZE 512
#define NUM_PHYSICAL_PAGES (MEMSIZE / PAGESIZE) // 16 pages
#define NUM_LOGICAL_PAGES 10 // 2 for Instruction, 8 for Data

// Global shared physical memory[cite: 1]
extern char memory[MEMSIZE]; 

// Logical buffers for OS loader[cite: 1]
extern char Instruction[MAX_PROC][256];
extern char Data[MAX_PROC][4096];

// Helper functions for OS
void load_memory(int proc_id, const char *prog_file, const char *data_file);
void save_memory(int proc_id, const char *data_file);

#endif