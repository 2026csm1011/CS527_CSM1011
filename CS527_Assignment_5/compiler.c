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
    if (strcmp(suffix, "NE") == 0) return 0x1;
    if (strcmp(suffix, "GE") == 0) return 0xA;
    return 0xE; // Default to AL (Always)
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

        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL && line[0] == '.') 
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
    
    FILE *out = fopen(out_filename, "w");
    current_line = 0;

    // PASS 2: Generate Bytecode
    while (fgets(line, sizeof(line), file)) 
    {
        char *comment = strchr(line, '%');
        if (comment) *comment = '\0';
        trim(line);
        if (strlen(line) == 0) continue;
        if (strchr(line, '=') == NULL && strchr(line, ' ') == NULL && line[0] == '.') continue;

        int op = 0, d = 0, s1 = 0, s2 = 0;
        char dest_str[20] = {0}, s1_str[20] = {0}, s2_str[20] = {0}, op_char = 0;

        // Print Instruction
        if (strncmp(line, "Print", 5) == 0 || strncmp(line, "print", 5) == 0) 
        {
            sscanf(line, "%*s x%d", &s2);
            op = 0x08;
            d = 0; s1 = 0; 
        }
        // Branch Instructions
        else if (line[0] == 'B') 
        {
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
            op = 0x10 + get_branch_suffix(suffix); 
            s2 = (unsigned char)offset;
        }
        // Legacy Read/Write
        else if (strncmp(line, "Read", 4) == 0) { sscanf(line, "Read x%d %d", &d, &s1); op = 0x05; } 
        else if (strncmp(line, "Write", 5) == 0) { sscanf(line, "Write x%d %d", &d, &s1); op = 0x06; }
        
        // Lab 5 Bracket Syntax: Memory Write
        else if (line[0] == '[') 
        {
            char mem_addr[20], src_val[20];
            if (sscanf(line, "[%[^]]] = %s", mem_addr, src_val) == 2) 
            {
                int is_vector = (src_val[0] == 'v');
                d = atoi(&src_val[1]); 
                if (mem_addr[0] == 'x') {
                    s2 = atoi(&mem_addr[1]);
                    op = is_vector ? 0x26 : 0x06; 
                } else {
                    s2 = atoi(mem_addr);
                    op = is_vector ? 0x2E : 0x0E; 
                }
                s1 = 0; 
            }
        }
        // Lab 5 Bracket Syntax: Memory Read
        else if (strchr(line, '[')) 
        {
            char mem_addr[20];
            if (sscanf(line, "%s = [%[^]]]", dest_str, mem_addr) == 2) 
            {
                int is_vector = (dest_str[0] == 'v');
                d = atoi(&dest_str[1]); 
                if (mem_addr[0] == 'x') {
                    s2 = atoi(&mem_addr[1]);
                    op = is_vector ? 0x25 : 0x05; 
                } else {
                    s2 = atoi(mem_addr);
                    op = is_vector ? 0x2C : 0x0C; 
                }
                s1 = 0; 
            }
        }
        // Lab 5 Math Operations (Integer & Vector)
        else if (sscanf(line, "%s = %s %c %s", dest_str, s1_str, &op_char, s2_str) == 4) 
        {
            int is_vector = (dest_str[0] == 'v');
            d = atoi(&dest_str[1]);
            s1 = atoi(&s1_str[1]);
            
            int is_s2_var = (s2_str[0] == 'x' || s2_str[0] == 'v');
            s2 = is_s2_var ? atoi(&s2_str[1]) : atoi(s2_str);

            if (op_char == '+') op = is_vector ? (is_s2_var ? 0x21 : 0x29) : (is_s2_var ? 0x01 : 0x09);
            else if (op_char == '-') op = is_vector ? (is_s2_var ? 0x22 : 0x2A) : (is_s2_var ? 0x02 : 0x0A);
            else if (op_char == '*') op = is_vector ? (is_s2_var ? 0x23 : 0x2B) : (is_s2_var ? 0x03 : 0x0B);
            else if (op_char == '/') op = is_vector ? (is_s2_var ? 0x24 : 0x2C) : (is_s2_var ? 0x04 : 0x0C); 
        }
        // Data Movement (x1 = 10 or x1 = x2)
        else if (sscanf(line, "%s = %s", dest_str, s1_str) == 2) 
        {
            d = atoi(&dest_str[1]);
            if (s1_str[0] == 'x' || s1_str[0] == 'v') {
                s2 = atoi(&s1_str[1]);
                op = 0x07; 
            } else {
                s2 = atoi(s1_str);
                op = 0x0F; 
            }
            s1 = 0; 
        }

        // Output exactly 4 hex bytes per line
        fprintf(out, "%02X %02X %02X %02X\n", op & 0xFF, d & 0xFF, s1 & 0xFF, s2 & 0xFF);
        current_line++;
    }
    
    // Halt instruction
    fprintf(out, "00 00 00 00\n"); 
    fclose(file);
    fclose(out);
}