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
char proc_out_filename[NP][100]; // Track auto-generated output filename per CPU
int next_pid = 1;
int end_of_simulation = 0;

// Shell State
int shell_active = 1;
char shell_buffer[256];
int shell_idx = 0;

// LAB 5 MMU Variables
char pageTable[MAX_PROC][NUM_LOGICAL_PAGES];
char freePages[NUM_PHYSICAL_PAGES] = {0};

// LAB 5 MMU Functions
int getFreePage() {
    for (int i = 1; i < NUM_PHYSICAL_PAGES; i++) {
        if (freePages[i] == 0) {
            freePages[i] = 1;
            return i;
        }
    }
    printf("\n[MMU Error] Out of physical memory frames!\n");
    exit(1); 
}

int getPhysicallAddress(int proc_id, int isFetch, int address) {
    int index = (isFetch) ? (address / PAGESIZE) : (address / PAGESIZE + (1024 / PAGESIZE));
    int phys_page = pageTable[proc_id][index];
    return (phys_page * PAGESIZE) + (address % PAGESIZE);
}

void os_init() 
{
    // CHANGED: Use "a" (Append) so old logs are saved permanently!
    FILE *log = fopen("execution.log", "a");
    if (log) {
        fprintf(log, "\n========================================\n");
        fprintf(log, "       NEW OS BOOT SESSION STARTED      \n");
        fprintf(log, "========================================\n");
        fclose(log);
    }
}

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
        
        // --- AUTO-GENERATE COMPILED PROGRAM FILENAME ---
        // Converts "test1.txt" into "test1_prog.byte"
        char prog_byte[100];
        strcpy(prog_byte, prog_file);
        char *dot_prog = strrchr(prog_byte, '.');
        if (dot_prog) *dot_prog = '\0';
        strcat(prog_byte, "_prog.byte");
        // -----------------------------------------------

        compile(prog_file, prog_byte);
        
        reset_processor(free_proc);
        load_memory(free_proc, prog_byte, data_file);
        
        // LAB 5 MMU INITIALIZE
        int inst_frame = getFreePage();
        pageTable[free_proc][0] = inst_frame;
        memcpy(&memory[inst_frame * PAGESIZE], Instruction[free_proc], 256);
        
        for (int p = 0; p < 8; p++) {
            int data_frame = getFreePage();
            pageTable[free_proc][2 + p] = data_frame;
            memcpy(&memory[data_frame * PAGESIZE], &Data[free_proc][p * PAGESIZE], PAGESIZE);
        }

        // --- AUTO-GENERATE OUTPUT FILENAME ---
        // Converts "test1.txt" into "test1_out.byte"
        char auto_out_file[100];
        strcpy(auto_out_file, prog_file);
        char *dot_out = strrchr(auto_out_file, '.'); 
        if (dot_out) *dot_out = '\0';
        strcat(auto_out_file, "_out.byte");
        
        strcpy(proc_out_filename[free_proc], auto_out_file);
        // -------------------------------------

        printf("\n[OS Loader] Compiled '%s' to '%s'\n", prog_file, prog_byte);
        printf("[OS Loader] Loaded data '%s' -> Output will save to '%s' (CPU %d)\n", 
                data_file, auto_out_file, free_proc);
        
        proc_pid[free_proc] = pid;
        proc_active[free_proc] = 1;
    } 
    else 
    {
        printf("\n[OS Loader] CPUs are full. '%s' added to Waiting Queue.\n", prog_file);
        strcpy(waiting_queue_prog[wait_tail], prog_file);
        strcpy(waiting_queue_data[wait_tail], data_file);
        wait_tail++;
    }
}

void shell() {
    if (!shell_active) return;

    if (_kbhit()) 
    {
        char c = _getch();
        
        if (c == '\r' || c == '\n') {
            shell_buffer[shell_idx] = '\0';
            
            char temp_buf[256];
            strcpy(temp_buf, shell_buffer);
            
            char *p_file = strtok(temp_buf, " \t");
            char *d_file = strtok(NULL, " \t");
            
            if (p_file != NULL) 
            {
                if (strcmp(p_file, "exit") == 0) 
                {
                    shell_active = 0;
                    printf("\n[OS Shell] Shutting down CLI. Finishing active tasks...\n");
                } 
                else if (d_file != NULL) 
                {
                    loader(p_file, d_file);
                } 
                else 
                {
                    printf("\n[Shell Error] Usage: [program.txt] [input_data.byte]\n");
                }
            }
            
            if (shell_active) printf("\n$ ");
            shell_idx = 0;
            memset(shell_buffer, 0, sizeof(shell_buffer));
        } 
        else if (c == '\b') 
        { 
            if (shell_idx > 0) { shell_idx--; printf("\b \b"); }
        } 
        else 
        {
            shell_buffer[shell_idx++] = c; printf("%c", c);
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
                printf("\n[OS Scheduler] Process %d on CPU %d finished!\n", proc_pid[i], i);
                
                // LAB 5 MMU FINALIZE
                for (int p = 0; p < 8; p++) {
                    int data_frame = pageTable[i][2 + p];
                    if (data_frame != 0) {
                        memcpy(&Data[i][p * PAGESIZE], &memory[data_frame * PAGESIZE], PAGESIZE);
                    }
                }

                // Save using the auto-generated filename
                save_memory(i, proc_out_filename[i]);
                printf("[OS Output] Answers safely saved to: %s\n", proc_out_filename[i]);
                
                // Reset page table
                for (int p = 0; p < NUM_LOGICAL_PAGES; p++) {
                    int frame = pageTable[i][p];
                    if (frame != 0) {
                        freePages[frame] = 0;
                        pageTable[i][p] = 0;
                    }
                }
                
                proc_active[i] = 0;
                
                if (wait_head < wait_tail) 
                {
                    loader(waiting_queue_prog[wait_head], waiting_queue_data[wait_head]);
                    wait_head++;
                } 
                else if (shell_active) 
                {
                    printf("\n$ "); 
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