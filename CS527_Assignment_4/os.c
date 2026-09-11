#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include "os.h"
#include "compiler.h"
#include "processor.h"
#include "memory.h"

// OS Queues and State
#define MAX_WAIT 100
char waiting_queue_prog[MAX_WAIT][100];
char waiting_queue_data[MAX_WAIT][100];
int wait_head = 0, wait_tail = 0;

int proc_active[NP] = {0};
int proc_pid[NP] = {0};
int next_pid = 1;
int end_of_simulation = 0;

// Shell State
int shell_active = 1;
char shell_buffer[256];
int shell_idx = 0;

void os_init() 
{
    FILE *log = fopen("execution.log", "w");
    if (log) fclose(log);
}

// CHANGED: Loader now accepts both the program and the data files
void loader(const char *prog_file, const char *data_file) 
{
    int free_proc = -1;
    for (int i = 0; i < NP; i++) 
    {
        if (!proc_active[i]) { free_proc = i; break; }
    }

    if (free_proc != -1) 
    {
        int pid = next_pid++;
        
        // GENERATE UNIQUE COMPILED FILE PER CORE
        char prog_byte[50];
        sprintf(prog_byte, "program_proc%d.byte", free_proc);

        // Compile the user's text file into bytecode
        compile(prog_file, prog_byte);
        
        // Notice we REMOVED the fopen("...data_proc...", "a") line here!
        // We will just use the data_file the user typed in the shell.

        printf("\n[OS Loader] Loaded '%s' and data '%s' into Processor %d (PID: %d)\n", prog_file, data_file, free_proc, pid);
        
        reset_processor(free_proc);
        
        // Pass the user's dynamically typed data file straight to memory
        load_memory(free_proc, prog_byte, data_file);
        
        proc_pid[free_proc] = pid;
        proc_active[free_proc] = 1;
        if (shell_active) printf("$ ");
    } 
    else 
    {
        printf("\n[OS Loader] CPUs are full. '%s' added to Waiting Queue.\n", prog_file);
        strcpy(waiting_queue_prog[wait_tail], prog_file);
        strcpy(waiting_queue_data[wait_tail], data_file);
        wait_tail++;
        if (shell_active) printf("$ ");
    }
}

void shell() {
    if (!shell_active) return;

    if (_kbhit()) 
    {
        char c = _getch();
        
        if (c == '\r' || c == '\n') {
            shell_buffer[shell_idx] = '\0';
            
            if (strcmp(shell_buffer, "exit") == 0) 
            {
                shell_active = 0;
                printf("\n[OS Shell] Shutting down CLI. Finishing active tasks...\n");
            } 
            else if (shell_idx > 0) 
            {
                // CHANGED: Copy the buffer so strtok doesn't destroy the original text
                char temp_buf[256];
                strcpy(temp_buf, shell_buffer);
                
                // Tokenize to find both files separated by a space
                char *p_file = strtok(temp_buf, " \t");
                char *d_file = strtok(NULL, " \t");
                
                if (p_file != NULL && d_file != NULL) {
                    loader(p_file, d_file);
                } else {
                    printf("\n[Shell Error] Usage: [program.txt] [data.byte]\n");
                    printf("$ ");
                }
            } 
            else 
            {
                printf("\n$ ");
            }
            shell_idx = 0;
        } 
        else if (c == '\b') 
        { 
            if (shell_idx > 0) 
            {
                shell_idx--;
                printf("\b \b");
            }
        } 
        else 
        {
            shell_buffer[shell_idx++] = c;
            printf("%c", c);
        }
    }
}

void scheduler() {
    int any_active = 0;

    for (int i = 0; i < NP; i++) 
    {
        if (proc_active[i]) 
        {
            any_active = 1;
            
            process_instructions(i, proc_pid[i], 10);
            
            if (core_halted[i]) 
            {
                printf("\n[OS Scheduler] Process %d on Processor %d finished!\n", proc_pid[i], i);
                
                // Save out to unique file
                char outfile[50];
                sprintf(outfile, "data_out_proc%d.byte", i);
                save_memory(i, outfile);
                
                proc_active[i] = 0;
                
                // CHANGED: Pull both files from the upgraded queue
                if (wait_head < wait_tail) 
                {
                    loader(waiting_queue_prog[wait_head], waiting_queue_data[wait_head]);
                    wait_head++;
                } 
                else if (shell_active) 
                {
                    printf("$ %s", shell_buffer); 
                }
            }
        }
    }
    
    if (!shell_active && !any_active && wait_head == wait_tail) 
    {
        end_of_simulation = 1;
    }
}

void os_run() 
{
    while (!end_of_simulation) 
    {
        scheduler();
        shell();
        Sleep(50);
    }
    printf("\n[OS] All processes complete. System shutting down safely.\n");
}