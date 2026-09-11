#include <stdio.h>
#include "os.h"

int main(int argc, char** argv) {
    printf("\n========================================\n");
    printf("     CS527 Multi-Core OS Simulator      \n");
    printf("========================================\n");
    
    os_init();

    // If you run: ./simulator prog1.txt prog2.txt, it loads them immediately
    for (int i = 1; i < argc; i++) {
        loader(argv[i]);
    }
    
    printf("\nWelcome to the OS Shell. Type a filename (e.g., prog1A.txt) to run it.\n");
    printf("Type 'exit' to shut down the shell.\n\n");
    printf("$ ");

    // Hand over control to the OS
    os_run();

    return 0;
}