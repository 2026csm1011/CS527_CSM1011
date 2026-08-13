#include <stdio.h>
#include "processor.h"
#include "memory.h"

int Register[256];
int PC = 0, opcode = 0, dest = 0, src1 = 0, src2 = 0;
int end_of_simulation = 0;

void reset() {
    for (int i = 0; i < 256; i++) {
        Register[i] = 0;
    }
    PC = 0;
    end_of_simulation = 0;
}

void fetch() {
    opcode = Instruction[PC];
    dest = Instruction[PC + 1];
    src1 = Instruction[PC + 2];
    src2 = Instruction[PC + 3];
    PC += 4;
}

void decode() {}

void execute() {
    if (opcode == 0) {
        end_of_simulation = 1;
        return;
    }

    printf("Opcode: %d | Dest: %d | Src1: %d | Src2: %d\n", opcode, dest, src1, src2);

    switch (opcode) {
        case 1:
            Register[dest] = Register[src1] + Register[src2];
            printf("Reg[%d] = %d\n", dest, Register[dest]);
            break;
        case 2:
            Register[dest] = Register[src1] - Register[src2];
            printf("Reg[%d] = %d\n", dest, Register[dest]);
            break;
        case 3:
            Register[dest] = Register[src1] * Register[src2];
            printf("Reg[%d] = %d\n", dest, Register[dest]);
            break;
        case 4:
            if (Register[src2] != 0) {
                Register[dest] = Register[src1] / Register[src2];
                printf("Reg[%d] = %d\n", dest, Register[dest]);
            }
            break;
        case 5:
            Register[dest] = Data[src1];
            printf("Reg[%d] loaded from memory\n", dest);
            break;
        case 6:
            Data[src1] = Register[dest];
            printf("Memory[%d] = %d\n", src1, Register[dest]);
            break;
        case 7:
            Register[dest] = src1;
            printf("Reg[%d] = %d\n", dest, Register[dest]);
            break;
    }
    printf("-------------------\n");
}