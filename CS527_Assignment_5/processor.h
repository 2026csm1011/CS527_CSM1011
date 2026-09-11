#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdint.h>
#include "memory.h" // Includes the NP (Number of Processors) definition

// 256 integer registers (x0 to x255) each 32 bit wide
extern int32_t Register[NP][256]; 

// 32 vector registers (v0 to v31) each 256 bits wide (8 integers of 32 bits)[cite: 1]
extern int32_t VReg[NP][32][8];

extern int PC[NP];
extern int core_halted[NP];

// Processor functions
void reset_processor(int proc_id);
void process_instructions(int proc_id, int pid, int slice_count);

#endif