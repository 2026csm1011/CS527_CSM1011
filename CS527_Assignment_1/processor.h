#ifndef PROCESSOR_H
#define PROCESSOR_H
extern int Register[256];
extern int PC, opcode, dest, src1, src2;
extern int end_of_simulation;
void reset();
void fetch();
void decode();
void execute();
#endif