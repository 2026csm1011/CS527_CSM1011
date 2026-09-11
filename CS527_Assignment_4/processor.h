#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <stdint.h>
#include "memory.h"

extern int32_t Register[NP][256]; 
extern int32_t VReg[NP][8][8];
extern int PC[NP];
extern int core_halted[NP]; // 1 if finished, 0 if running

void reset_processor(int proc_id);

// The new multi-tasking executor
void process_instructions(int proc_id, int pid, int slice_count);

#endif