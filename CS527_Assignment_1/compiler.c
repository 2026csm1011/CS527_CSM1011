#include <stdio.h>
#include "compiler.h"

void compile() {
    int choice;
    char *filename = "";
    char *readable_name = "";
    
    printf("Test programs:\n");
    printf("1 - Sum of N numbers\n");
    printf("2 - Multiply two complex numbers\n");
    printf("3 - Determinant of a 3x3 matrix\n");
    printf("Enter choice (1-3): ");
    
    scanf("%d", &choice);

    switch(choice) {
        case 1:
            filename = "faq1.txt";
            readable_name = "readable_faq1.txt";
            break;
        case 2:
            filename = "faq2.txt";
            readable_name = "readable_faq2.txt";
            break;
        case 3:
            filename = "faq3.txt";
            readable_name = "readable_faq3.txt";
            break;
        default:
            printf("Invalid choice.\n");
            return; 
    }

    // Open the source text file for reading
    FILE *txt_file = fopen(filename, "r");
    
    // Open the binary file using the absolute path to ensure correct placement
    FILE *byte_file = fopen("C:\\Users\\Indrasen Gupta\\Desktop\\CS527_Assignment_1\\program.byte", "wb");
    
    // Open the readable text file for human verification
    FILE *readable_file = fopen(readable_name, "w");

    if (txt_file != NULL && byte_file != NULL && readable_file != NULL) {
        int op, d, s1, s2;
        
        while (fscanf(txt_file, "%d %d %d %d", &op, &d, &s1, &s2) == 4) {
            // Write raw binary data for the CPU execution
            char instr[4] = {(char)op, (char)d, (char)s1, (char)s2};
            fwrite(instr, 1, 4, byte_file);
            
            // Write formatted text data for human readability
            fprintf(readable_file, "%d %d %d %d\n", op, d, s1, s2);
        }
        
        fclose(txt_file);
        fclose(byte_file);
        fclose(readable_file);
        
        printf("\nCompiled %s successfully!\n", filename);
        printf("To view the compiled instructions, open '%s' in your editor.\n\n", readable_name);
    } else {
        printf("Error: Could not open files.\n");
    }
}