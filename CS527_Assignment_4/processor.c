#include <stdio.h>
#include <stdint.h>
#include "processor.h"

int32_t Register[NP][256]; 
int32_t VReg[NP][8][8];
int PC[NP];
int core_halted[NP];

void reset_processor(int proc_id) 
{
    if (proc_id < 0 || proc_id >= NP) return;
    for (int i = 0; i < 256; i++) Register[proc_id][i] = 0; 
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) 
        {
            VReg[proc_id][i][j] = 0;
        }
    }
    PC[proc_id] = 0; 
    core_halted[proc_id] = 0;
}

void process_instructions(int proc_id, int pid, int slice_count) 
{
    if (proc_id < 0 || proc_id >= NP || core_halted[proc_id]) return;

    for (int step = 0; step < slice_count; step++) {
        // --- FETCH ---
        int current_pc = PC[proc_id];
        if (current_pc < 0 || current_pc >= 252) 
        {
            core_halted[proc_id] = 1;
            return;
        }

        int opcode = Instruction[proc_id][current_pc];
        int dest = Instruction[proc_id][current_pc + 1];
        int src1 = Instruction[proc_id][current_pc + 2];
        int src2 = Instruction[proc_id][current_pc + 3];
        PC[proc_id] += 4;

        // --- DECODE & EXECUTE ---
        if (opcode == 0) 
        {
            core_halted[proc_id] = 1; 
            return; // Exit time slice early if finished
        }

        switch (opcode) 
        {
            case 0x01: Register[proc_id][dest] = Register[proc_id][src1] + Register[proc_id][src2]; break;
            case 0x02: Register[proc_id][dest] = Register[proc_id][src1] - Register[proc_id][src2]; break;
            case 0x03: Register[proc_id][dest] = Register[proc_id][src1] * Register[proc_id][src2]; break;
            
            case 0x09: Register[proc_id][dest] = Register[proc_id][src1] + src2; break;
            case 0x0A: Register[proc_id][dest] = Register[proc_id][src1] - src2; break;
            case 0x0B: Register[proc_id][dest] = Register[proc_id][src1] * src2; break;

            case 0x05: Register[proc_id][dest] = Data[proc_id][src1]; break;
            case 0x06: Data[proc_id][src1] = Register[proc_id][dest]; break;
            case 0x07: Register[proc_id][dest] = src1; break;

            // NEW: Print Instruction (Opcode 0x08)
            case 0x08: 
            {
                FILE *log = fopen("execution.log", "a");
                if (log) 
                {
                    fprintf(log, "Process id: %d x%d : 0x%08X\n", pid, src1, Register[proc_id][src1]);
                    fclose(log);
                }
                break;
            }

            case 0x10: 
                if (Register[proc_id][6] == 0) PC[proc_id] = (PC[proc_id] - 4) + (((signed char)src2) * 4);
                break;
            case 0x1E: 
                PC[proc_id] = (PC[proc_id] - 4) + (((signed char)src2) * 4);
                break;

            // SIMD Vector Operations
            case 0x20:
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = Data[proc_id][Register[proc_id][src1] + (i * 4)];
                break;
            case 0x21:
                for (int i = 0; i < 8; i++) Data[proc_id][Register[proc_id][src1] + (i * 4)] = VReg[proc_id][dest][i];
                break;
            case 0x22:
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] + VReg[proc_id][src2][i];
                break;
            case 0x23:
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] * VReg[proc_id][src2][i];
                break;
            case 0x24:
                Register[proc_id][dest] = 0;
                for (int i = 0; i < 8; i++) Register[proc_id][dest] += VReg[proc_id][src1][i];
                break;
            case 0x25:
                Data[proc_id][Register[proc_id][src1]] = Register[proc_id][dest];
                break;
        }
    }
}