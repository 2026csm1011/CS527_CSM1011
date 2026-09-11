#include <stdio.h>
#include <stdint.h>
#include "processor.h"
#include "memory.h" 

int32_t Register[NP][256]; 
int32_t VReg[NP][32][8]; 
int PC[NP];
int core_halted[NP];

int getPhysicallAddress(int proc_id, int isFetch, int address);

// HELPER: Read 4 bytes from physical memory and combine into a 32-bit integer[cite: 1]
int32_t read_32bit(int phys_addr) {
    return (unsigned char)memory[phys_addr] |
          ((unsigned char)memory[phys_addr + 1] << 8) |
          ((unsigned char)memory[phys_addr + 2] << 16) |
          ((unsigned char)memory[phys_addr + 3] << 24);
}

// HELPER: Break a 32-bit integer into 4 bytes and write to physical memory[cite: 1]
void write_32bit(int phys_addr, int32_t val) {
    memory[phys_addr]     = val & 0xFF;
    memory[phys_addr + 1] = (val >> 8) & 0xFF;
    memory[phys_addr + 2] = (val >> 16) & 0xFF;
    memory[phys_addr + 3] = (val >> 24) & 0xFF;
}

void reset_processor(int proc_id) 
{
    if (proc_id < 0 || proc_id >= NP) return;
    for (int i = 0; i < 256; i++) Register[proc_id][i] = 0; 
    for (int i = 0; i < 32; i++) {
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
        int current_pc = PC[proc_id];
        if (current_pc < 0 || current_pc >= 256) 
        {
            core_halted[proc_id] = 1;
            return;
        }

        int p_addr_0 = getPhysicallAddress(proc_id, 1, current_pc);
        int p_addr_1 = getPhysicallAddress(proc_id, 1, current_pc + 1);
        int p_addr_2 = getPhysicallAddress(proc_id, 1, current_pc + 2);
        int p_addr_3 = getPhysicallAddress(proc_id, 1, current_pc + 3);

        int opcode = (unsigned char)memory[p_addr_0];
        int dest   = (unsigned char)memory[p_addr_1];
        int src1   = (unsigned char)memory[p_addr_2];
        int src2   = (unsigned char)memory[p_addr_3];
        
        PC[proc_id] += 4;

        if (opcode == 0) 
        {
            core_halted[proc_id] = 1; 
            return; 
        }

        switch (opcode) 
        {
            case 0x01: Register[proc_id][dest] = Register[proc_id][src1] + Register[proc_id][src2]; break;
            case 0x02: Register[proc_id][dest] = Register[proc_id][src1] - Register[proc_id][src2]; break;
            case 0x03: Register[proc_id][dest] = Register[proc_id][src1] * Register[proc_id][src2]; break;
            case 0x04: if(Register[proc_id][src2] != 0) Register[proc_id][dest] = Register[proc_id][src1] / Register[proc_id][src2]; break;
            
            case 0x09: Register[proc_id][dest] = Register[proc_id][src1] + src2; break;
            case 0x0A: Register[proc_id][dest] = Register[proc_id][src1] - src2; break;
            case 0x0B: Register[proc_id][dest] = Register[proc_id][src1] * src2; break;
            case 0x0C: if(src2 != 0) Register[proc_id][dest] = Register[proc_id][src1] / src2; break;

            case 0x05: case 0x0D: // Scalar Memory Read 
            {
                int logical_addr = (opcode == 0x05) ? Register[proc_id][src2] : src2;
                int phys_data_addr = getPhysicallAddress(proc_id, 0, logical_addr);
                Register[proc_id][dest] = read_32bit(phys_data_addr); // Read 4 bytes[cite: 1]
                break;
            }
            case 0x06: case 0x0E: // Scalar Memory Write
            {
                int logical_addr = (opcode == 0x06) ? Register[proc_id][src2] : src2;
                int phys_data_addr = getPhysicallAddress(proc_id, 0, logical_addr);
                write_32bit(phys_data_addr, Register[proc_id][dest]); // Write 4 bytes[cite: 1]
                break;
            }
            case 0x07: case 0x0F: // Data Movement
                Register[proc_id][dest] = (opcode == 0x07) ? Register[proc_id][src2] : src2; 
                break;

            case 0x08: 
            {
                FILE *log = fopen("execution.log", "a");
                if (log) 
                {
                    fprintf(log, "Process id: %d x%d : 0x%08X\n", pid, src2, Register[proc_id][src2]);
                    fclose(log);
                }
                break;
            }

            case 0x10: 
            case 0x1E: 
                PC[proc_id] = (PC[proc_id] - 4) + (((signed char)src2) * 4);
                break;

            case 0x21: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] + VReg[proc_id][src2][i];
                break;
            case 0x22: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] - VReg[proc_id][src2][i];
                break;
            case 0x23: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] * VReg[proc_id][src2][i];
                break;
            case 0x29: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] + src2;
                break;
            case 0x2A: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] - src2;
                break;
            case 0x2B: 
                for (int i = 0; i < 8; i++) VReg[proc_id][dest][i] = VReg[proc_id][src1][i] * src2;
                break;

            case 0x25: case 0x2C: // Vector Memory Read
            {
                int base_addr = (opcode == 0x25) ? Register[proc_id][src2] : src2;
                for (int i = 0; i < 8; i++) {
                    int logical_addr = base_addr + (i * 4);
                    int phys_addr = getPhysicallAddress(proc_id, 0, logical_addr);
                    VReg[proc_id][dest][i] = read_32bit(phys_addr); // Read 4 bytes per vector element[cite: 1]
                }
                break;
            }
            case 0x26: case 0x2E: // Vector Memory Write
            {
                int base_addr = (opcode == 0x26) ? Register[proc_id][src2] : src2;
                for (int i = 0; i < 8; i++) {
                    int logical_addr = base_addr + (i * 4);
                    int phys_addr = getPhysicallAddress(proc_id, 0, logical_addr);
                    write_32bit(phys_addr, VReg[proc_id][dest][i]); // Write 4 bytes per vector element[cite: 1]
                }
                break;
            }
        }
    }
}