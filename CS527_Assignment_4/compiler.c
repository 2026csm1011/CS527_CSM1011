#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

struct Label { char name[50]; int line_num; };
struct Label labels[100];
int label_count = 0;

void trim(char *str) 
{
    char *p = str;
    int l = strlen(p);
    while(l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while(*p && isspace(*p)) ++p, --l;
    memmove(str, p, l + 1);
}

int get_branch_suffix(char *suffix) 
{
    if (strcmp(suffix, "EQ") == 0) return 0x0;
    return 0xE; 
}

void compile(const char *filename, const char *out_filename) 
{
    FILE *file = fopen(filename, "r");
    if (!file) 
    {
        printf("\n[Compiler Error] Cannot open '%s'\n", filename);
        return;
    }

    label_count = 0;
    char line[256];
    int current_line = 0;

    // PASS 1: Identify Labels
    while (fgets(line, sizeof(line), file)) 
    {
        char *comment = strchr(line, '%');
        if (comment) *comment = '\0'; 
        trim(line);
        if (strlen(line) == 0) continue;

        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL) 
        {
            char *colon = strchr(line, ':');
            if (colon) *colon = '\0';
            strcpy(labels[label_count].name, line);
            labels[label_count].line_num = current_line;
            label_count++;
        } 
        else 
        {
            current_line++; 
        }
    }

    rewind(file);
    
    // NOW OPENS THE UNIQUE PROCESSOR FILE
    FILE *out = fopen(out_filename, "w");
    current_line = 0;

    // PASS 2: Generate Bytecode
    while (fgets(line, sizeof(line), file)) 
    {
        char *comment = strchr(line, '%');
        if (comment) *comment = '\0';
        trim(line);
        if (strlen(line) == 0) continue;
        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL) continue;

        int op = 0, d = 0, s1 = 0, s2 = 0;

        // Print Instruction
        if (strncmp(line, "Print", 5) == 0) 
        {
            sscanf(line, "Print x%d", &s1);
            op = 0x08;
        }
        else if (strncmp(line, "VLD", 3) == 0) { sscanf(line, "VLD v%d x%d", &d, &s1); op = 0x20; } 
        else if (strncmp(line, "VST", 3) == 0) { sscanf(line, "VST v%d x%d", &d, &s1); op = 0x21; }
        else if (strncmp(line, "VADD", 4) == 0){ sscanf(line, "VADD v%d v%d v%d", &d, &s1, &s2); op = 0x22; }
        else if (strncmp(line, "VMUL", 4) == 0){ sscanf(line, "VMUL v%d v%d v%d", &d, &s1, &s2); op = 0x23; }
        else if (strncmp(line, "VSUM", 4) == 0){ sscanf(line, "VSUM x%d v%d", &d, &s1); op = 0x24; }
        else if (strncmp(line, "SST", 3) == 0) { sscanf(line, "SST x%d x%d", &d, &s1); op = 0x25; }
        else if (line[0] == 'B') {
            char suffix[3] = {0}, lbl[50] = {0};
            if (sscanf(line, "B%2s %s", suffix, lbl) != 2) sscanf(line, "B%2s.%s", suffix, lbl);
            int offset = 0;
            for (int i = 0; i < label_count; i++) 
            {
                if (strcmp(labels[i].name, lbl) == 0) 
                { 
                    offset = labels[i].line_num - current_line; break; 
                }
            }
            op = 0x10 + get_branch_suffix(suffix); s2 = (unsigned char)offset;
        }
        else if (strncmp(line, "Read", 4) == 0)  
        { sscanf(line, "Read x%d %d", &d, &s1); op = 0x05; } 
        else if (strncmp(line, "Write", 5) == 0) 
        { sscanf(line, "Write x%d %d", &d, &s1); op = 0x06; }
        else 
        {
            char op_char = 0, s2_str[20];
            if (sscanf(line, "x%d = x%d %c %s", &d, &s1, &op_char, s2_str) == 4) 
            {
                if (s2_str[0] == 'x') sscanf(s2_str, "x%d", &s2);
                else sscanf(s2_str, "%d", &s2);
                if (op_char == '+') op = (s2_str[0] == 'x') ? 0x01 : 0x09;
                else if (op_char == '-') op = (s2_str[0] == 'x') ? 0x02 : 0x0A;
                else if (op_char == '*') op = (s2_str[0] == 'x') ? 0x03 : 0x0B;
            } 
            else if (sscanf(line, "x%d = %d", &d, &s1) == 2) op = 0x07; 
        }

        fprintf(out, "%d %d %d %d\n", op, d, s1, s2);
        current_line++;
    }
    fprintf(out, "0 0 0 0\n"); 
    fclose(file);
    fclose(out);
}