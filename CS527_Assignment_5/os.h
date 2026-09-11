#ifndef OS_H
#define OS_H

// Core OS Functions
void os_init();
void loader(const char *prog_file, const char *data_file); 
void scheduler();
void shell();
void os_run();

// Lab 5 MMU Functions
int getPhysicallAddress(int proc_id, int isFetch, int address);

#endif