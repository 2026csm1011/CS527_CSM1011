#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

struct Label {
    char name[50];
    int line_num; 
};

struct Label labels[100];
int label_count = 0;

void trim(char *str) {
    char *p = str;
    int l = strlen(p);
    while(isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;
    memmove(str, p, l + 1);
}

int get_branch_suffix(char *suffix) {
    if (strcmp(suffix, "EQ") == 0) return 0x0;
    if (strcmp(suffix, "AL") == 0) return 0xE; 
    return 0xE; 
}

void compile() {
    int choice;
    char filename[100] = "";
    
    // --- faq menu option---
    printf("\n========================================\n");
    printf("        CS527 LAB 2 SIMULATOR           \n");
    printf("========================================\n");
    printf("Test programs:\n");
    printf("1 - Sum of N numbers (fixed at program time)\n");
    printf("2 - Multiply two complex numbers\n");
    printf("3 - Determinant of a 3x3 matrix\n");
    printf("4 - Sum of N numbers (given at run time)\n");
    printf("5 - FIR filter response\n");
    printf("========================================\n");
    printf("Enter choice (1-5): ");
    
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        return;
    }

    switch(choice) {
        case 1: strcpy(filename, "faq1.txt"); break;
        case 2: strcpy(filename, "faq2.txt"); break;
        case 3: strcpy(filename, "faq3.txt"); break;
        case 4: strcpy(filename, "faq4.txt"); break;
        case 5: strcpy(filename, "faq5.txt"); break;
        default:
            printf("Invalid choice. Exiting.\n");
            return; 
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("\n[Compiler Error] Cannot open '%s'. Did you create this file in the folder?\n", filename);
        return;
    }

    char line[256];
    int current_line = 0;

    // PASS 1: Identify Labels
    while (fgets(line, sizeof(line), file)) {
        char *comment = strchr(line, '%');
        if (comment) *comment = '\0'; 
        trim(line);
        if (strlen(line) == 0) continue;

        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL && strchr(line, '.') == NULL) {
    
            char *colon = strchr(line, ':');
            if (colon) *colon = '\0';
            
            strcpy(labels[label_count].name, line);
            labels[label_count].line_num = current_line;
            label_count++;
        } else {
            current_line++; 
        }
    }

    rewind(file);
    
    // DYNAMIC PATH -
    FILE *out = fopen("program.byte", "w");
    current_line = 0;

    // PASS 2: Generate Bytecode
    while (fgets(line, sizeof(line), file)) {
        char *comment = strchr(line, '%');
        if (comment) *comment = '\0';
        trim(line);
        if (strlen(line) == 0) continue;

        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL && strchr(line, '.') == NULL) continue;

        int op = 0, d = 0, s1 = 0, s2 = 0;

        if (line[0] == 'B') {
            char suffix[3] = {0};
            char lbl[50] = {0};
            
            if (sscanf(line, "B%2s.%s", suffix, lbl) != 2) {
                sscanf(line, "B%2s %s", suffix, lbl);
            }
            
            int offset = 0;
            int found = 0;
            for (int i = 0; i < label_count; i++) {
                if (strcmp(labels[i].name, lbl) == 0) {
                    offset = labels[i].line_num - current_line; 
                    found = 1;
                    break;
                }
            }
            
            // critical safety check
            if (!found) {
                printf("\n[FATAL ERROR] The compiler could not find the label '%s' in %s!\n", lbl, filename);
                printf("Check your file for typos around '%s'\n\n", lbl);
                exit(1); //stop infinite loop
            }

            op = 0x10 + get_branch_suffix(suffix); 
            d = 0; s1 = 0; s2 = (unsigned char)offset;
        }
        else if (strchr(line, '[')) {
            if (line[0] == '[') {
                char d_str[10], s_str[10];
                sscanf(line, "[%[^]]] = %s", d_str, s_str);
                sscanf(d_str, "x%d", &d);
                if (s_str[0] == 'x') {
                    sscanf(s_str, "x%d", &s2);
                    op = 0x06; 
                } else {
                    sscanf(s_str, "%d", &s2);
                    op = 0x0E; 
                }
            } else {
                char d_str[10], s_str[10];
                sscanf(line, "x%d = [%[^]]]", &d, s_str);
                if (s_str[0] == 'x') {
                    sscanf(s_str, "x%d", &s2);
                    op = 0x05; 
                } else {
                    sscanf(s_str, "%d", &s2);
                    op = 0x0D; 
                }
            }
        }
        else if (strncmp(line, "Read", 4) == 0) {
            sscanf(line, "Read x%d %d", &d, &s2);
            op = 0x0D; 
        } else if (strncmp(line, "Write", 5) == 0) {
            sscanf(line, "Write x%d %d", &d, &s2);
            op = 0x0E; 
        }
        else {
            if (strchr(line, '=') == NULL) {
                sscanf(line, "x%dx%d-%s", &d, &s1, (char*)&s2);
                op = 0x02; 
                sscanf((char*)&s2, "x%d", &s2);
            } else {
                int items = 0;
                char op_char;
                char s2_str[10];
                items = sscanf(line, "x%d = x%d %c %s", &d, &s1, &op_char, s2_str);
                
                if (items == 4) {
                    int is_var = (s2_str[0] == 'x');
                    if (is_var) sscanf(s2_str, "x%d", &s2);
                    else sscanf(s2_str, "%d", &s2);

                    if (op_char == '+') op = is_var ? 0x01 : 0x09;
                    else if (op_char == '-') op = is_var ? 0x02 : 0x0A;
                    else if (op_char == '*') op = is_var ? 0x03 : 0x0B;
                    else if (op_char == '/') op = is_var ? 0x04 : 0x0C; 
                } else {
                    sscanf(line, "x%d = %d", &d, &s2);
                    op = 0x0F; 
                }
            }
        }

        fprintf(out, "%X %X %X %X\n", op, d, s1, (unsigned char)s2);
        current_line++;
    }

    fprintf(out, "0 0 0 0\n"); 
    
    fclose(file);
    fclose(out);
    printf("\n[Success] Compiled %s to program.byte\n", filename);
}