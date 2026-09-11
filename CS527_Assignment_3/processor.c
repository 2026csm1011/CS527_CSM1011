#include <stdio.h>
#include <stdint.h>
#include "processor.h"

int32_t Register[256]; 
int32_t VReg[8][8];
int PC;
int opcode, dest, src1, src2;
int end_of_simulation = 0;
int instruction_count = 0;

extern unsigned char Instruction[256];
extern unsigned char Data[4096]; 

void reset() {
    for (int i = 0; i < 256; i++) {
        Register[i] = 0; 
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            VReg[i][j] = 0;
        }
    }
    PC = 0; 
    end_of_simulation = 0;
    instruction_count = 0;
}

void fetch() {
    if (PC < 0 || PC >= 252) {
        printf("\n[CPU Error] Program Counter (PC) out of bounds: %d\n", PC);
        opcode = 0;
        return;
    }
    opcode = Instruction[PC];
    dest = Instruction[PC + 1];
    src1 = Instruction[PC + 2];
    src2 = Instruction[PC + 3];
    
    printf("[CPU Trace] PC=%d | Opcode=%d, Dest=%d, Src1=%d, Src2=%d\n", PC, opcode, dest, src1, src2);
    
    PC += 4;
}

void decode() {
    // Decoding is handled directly in execute stage
} 

void execute() {
    instruction_count++;
    
    // Safety check to prevent infinite loops
    if (instruction_count > 2000) {
        printf("\n[CPU Stop] Instruction limit reached. Forcing simulation exit.\n");
        end_of_simulation = 1;
        return;
    }

    if (opcode == 0) {
        printf("[CPU Status] Halt instruction (0x00) reached.\n");
        end_of_simulation = 1; 
        return;
    }

    switch (opcode) {
        case 0x01: // Add Register
            Register[dest] = Register[src1] + Register[src2];
            break;
        case 0x02: // Sub Register
            Register[dest] = Register[src1] - Register[src2];
            break;
        case 0x03: // Mul Register
            Register[dest] = Register[src1] * Register[src2];
            break;

        case 0x09: // Add Immediate
            Register[dest] = Register[src1] + src2;
            break;
        case 0x0A: // Sub Immediate
            Register[dest] = Register[src1] - src2;
            break;
        case 0x0B: // Mul Immediate
            Register[dest] = Register[src1] * src2;
            break;

        case 0x05: // Read Memory
            Register[dest] = Data[src1];
            break;
        case 0x06: // Write Memory
            Data[src1] = Register[dest];
            break;
        case 0x07: // Data Move Immediate
            Register[dest] = src1;
            break;

        case 0x10: // BEQ (Branch if Register[6] == 0)
            if (Register[6] == 0) {
                PC = (PC - 4) + (((signed char)src2) * 4);
            }
            break;
        case 0x1E: // BAL (Branch Always)
            PC = (PC - 4) + (((signed char)src2) * 4);
            break;

        // --- SIMD Vector Operations ---
        case 0x20: // VLD: Vector Load
            for (int i = 0; i < 8; i++) {
                VReg[dest][i] = Data[Register[src1] + (i * 4)];
            }
            break;
        case 0x21: // VST: Vector Store
            for (int i = 0; i < 8; i++) {
                Data[Register[src1] + (i * 4)] = VReg[dest][i];
            }
            break;
        case 0x22: // VADD: Vector Add
            for (int i = 0; i < 8; i++) {
                VReg[dest][i] = VReg[src1][i] + VReg[src2][i];
            }
            break;
        case 0x23: // VMUL: Vector Multiply
            for (int i = 0; i < 8; i++) {
                VReg[dest][i] = VReg[src1][i] * VReg[src2][i];
            }
            break;
        case 0x24: // VSUM: Vector Sum to Scalar
            Register[dest] = 0;
            for (int i = 0; i < 8; i++) {
                Register[dest] += VReg[src1][i];
            }
            break;
        case 0x25: // SST: Store Scalar to dynamic pointer
            Data[Register[src1]] = Register[dest];
            break;
    }
}