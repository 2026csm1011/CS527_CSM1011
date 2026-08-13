#ifndef PROCESSOR_H
#define PROCESSOR_H
#include <stdint.h>

extern int32_t Register[256]; 
extern int PC, opcode, dest, src1, src2;
extern int end_of_simulation;
extern int Z, N, C, V; 

void reset();
void fetch();
void decode();
void execute();

#endif