#include <stdio.h>
#include <stdint.h>
#include "processor.h"

int32_t Register[256]; 
int PC, opcode, dest, src1, src2;
int end_of_simulation = 0;
int Z = 0, N = 0, C = 0, V = 0; 
int instruction_count = 0;

extern unsigned char Instruction[256];
extern unsigned char Data[4096]; 

void reset() {
    for (int i = 0; i < 256; i++) Register[i] = 0; 
    PC = 0; 
    Z = 0; N = 0; C = 0; V = 0;
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
    
    //prints every instruction as it runs
    printf("[CPU Trace] PC=%d | Opcode=0x%02X, Dest=%d, Src1=%d, Src2=%d\n", PC, opcode, dest, src1, src2);
    
    PC += 4;
}

void decode() {} 

void execute() {
    instruction_count++;
    
    // Force stop if it exceeds 2000 instructions to prevent infinite loops
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

    int32_t operand1 = Register[src1];
    int32_t operand2 = 0;
    int current_inst_address = PC - 4;

    // Branches[cite: 1]
    if (opcode >= 0x10 && opcode <= 0x1E) {
        int branch_taken = 0;
        int condition = opcode - 0x10; 
        int8_t offset = (int8_t)src2; 

        switch (condition) {
            case 0x0: branch_taken = (Z == 1); break; // EQ[cite: 1]
            case 0x1: branch_taken = (Z == 0); break; // NE[cite: 1]
            case 0x2: branch_taken = (C == 1); break; // CS[cite: 1]
            case 0x3: branch_taken = (C == 0); break; // CC[cite: 1]
            case 0x4: branch_taken = (N == 1); break; // MI[cite: 1]
            case 0x5: branch_taken = (N == 0); break; // PL[cite: 1]
            case 0x6: branch_taken = (V == 1); break; // VS[cite: 1]
            case 0x7: branch_taken = (V == 0); break; // VC[cite: 1]
            case 0x8: branch_taken = (C == 1 && Z == 0); break; // HI[cite: 1]
            case 0x9: branch_taken = (C == 0 || Z == 1); break; // LS[cite: 1]
            case 0xA: branch_taken = (N == V); break; // GE[cite: 1]
            case 0xB: branch_taken = (N != V); break; // LT[cite: 1]
            case 0xC: branch_taken = (Z == 0 && N == V); break; // GT[cite: 1]
            case 0xD: branch_taken = (Z == 1 || N != V); break; // LE[cite: 1]
            case 0xE: branch_taken = 1; break; // AL[cite: 1]
        }
        if (branch_taken) {
            // UPDATED: MULTIPLY BY 4 because the offset is provided in instructions, not bytes![cite: 1]
            PC = current_inst_address + (offset * 4); 
        }
        return;
    }

    if (opcode <= 0x07) {
        operand2 = Register[src2];
    } else {
        operand2 = src2;
    }

    int32_t result = 0;
    int32_t addr = 0;

    switch (opcode) {
        case 0x01: // Add Var[cite: 1]
        case 0x09: // Add Const[cite: 1]
            result = operand1 + operand2;
            Register[dest] = result;
            Z = (result == 0) ? 1 : 0;
            N = (result < 0) ? 1 : 0;
            C = ((uint32_t)result < (uint32_t)operand1) ? 1 : 0;
            V = (((operand1 ^ result) & (operand2 ^ result)) < 0) ? 1 : 0;
            break;

        case 0x02: // Sub Var[cite: 1]
        case 0x0A: // Sub Const[cite: 1]
            result = operand1 - operand2;
            Register[dest] = result;
            Z = (result == 0) ? 1 : 0;
            N = (result < 0) ? 1 : 0;
            C = ((uint32_t)operand1 >= (uint32_t)operand2) ? 1 : 0;
            V = (((operand1 ^ operand2) & (operand1 ^ result)) < 0) ? 1 : 0;
            break;

        case 0x03: case 0x0B: // Mul[cite: 1]
            Register[dest] = operand1 * operand2;
            break;

        case 0x04: case 0x0C: // Div[cite: 1]
            if (operand2 != 0) Register[dest] = operand1 / operand2;
            break;

        case 0x05: case 0x0D: // Memory Read[cite: 1]
            addr = operand2; 
            if (addr < 0 || addr >= 4093) break; 
            Register[dest] = (Data[addr] << 24) | (Data[addr+1] << 16) | (Data[addr+2] << 8) | Data[addr+3];
            break;

        case 0x06: case 0x0E: // Memory Write[cite: 1]
            addr = Register[dest];
            if (addr < 0 || addr >= 4093) break;
            Data[addr] = (operand2 >> 24) & 0xFF;
            Data[addr+1] = (operand2 >> 16) & 0xFF;
            Data[addr+2] = (operand2 >> 8) & 0xFF;
            Data[addr+3] = operand2 & 0xFF;
            break;

        case 0x07: case 0x0F: // Data Move[cite: 1]
            Register[dest] = operand2;
            break;
    }
}