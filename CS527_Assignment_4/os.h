#ifndef OS_H
#define OS_H

void os_init();
void loader(const char *prog_file, const char *data_file); 
void scheduler();
void shell();
void os_run();

#endif