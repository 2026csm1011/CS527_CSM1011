#include <stdio.h>
#include "os.h"

int main(int argc, char** argv) {
    printf("\n========================================\n");
    printf("      CS527 Multi-Core OS Simulator     \n");
    printf("========================================\n");
    
    os_init();

    // If you run: ./simulator prog1.txt data1.byte, it loads them immediately
    // We step by 2 (i += 2) because every program requires its data file pair!
    for (int i = 1; i < argc; i += 2) {
        if (i + 1 < argc) {
            loader(argv[i], argv[i + 1]);
        } else {
            printf("[Error] Missing data file argument for program '%s'\n", argv[i]);
        }
    }
    
    printf("\nWelcome to the OS Shell. Type a program and data file (e.g., test1.txt input1.byte) to run it.\n");
    printf("Type 'exit' to shut down the shell.\n\n");

    // Hand over control to the OS
    os_run();

    return 0;
}