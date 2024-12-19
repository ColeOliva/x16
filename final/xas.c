#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include "instruction.h"

#define MAX_LINE_LENGTH 256
#define MAX_LABELS 100

typedef struct {
    char name[MAX_LINE_LENGTH];
    uint16_t address;
} Label;

Label labels[MAX_LABELS];
int num_labels = 0;

uint16_t current_address = DEFAULT_CODESTART;

void usage() {
    fprintf(stderr, "Usage: ./xas file\n");
    exit(1);
}

reg_t map_register(const char* reg_str) {
    if (reg_str[0] != '%') {
        fprintf(stderr, "Error: Invalid register reference '%s'\n", reg_str);
        exit(2);
    }
    int reg_num = atoi(reg_str + 2);
    if (reg_num < R_R0 || reg_num > R_R7) {
        fprintf(stderr, "Error: Invalid register number '%d'\n", reg_num);
        exit(2);
    }
    return (reg_t)reg_num;
}

uint16_t find_addy(char label_name[]) {
    for (int i = 0; i < num_labels; i++) {
        if (strcmp(labels[i].name, label_name) == 0) {
            return labels[i].address;
        }
    }
    exit(2);
}

void error_val(char operand[], FILE* input, FILE* output) {
    if (operand[0] != '$') {
        fprintf(stderr, "Error: Operand should start with '$'\n");
        fclose(input);
        fclose(output);
        exit(2);
    }
}

void parse_labels(FILE* input_file) {
    char line[MAX_LINE_LENGTH];
    uint16_t label_address = DEFAULT_CODESTART;
    while (fgets(line, sizeof(line), input_file) != NULL) {
        char* colon_ptr = strchr(line, ':');
        if (colon_ptr != NULL) {
            *colon_ptr = '\0';
            strcpy(labels[num_labels].name, line);
            labels[num_labels].address = label_address;
            num_labels++;
        } else {
            char* comment_ptr = strchr(line, '#');
            if (comment_ptr != NULL) {
                comment_ptr = '\0';
            }
            if (line[0] == '\0') {
                continue;
            }
            label_address += 2;
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        usage();
    }
    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error: Cannot open input file.\n");
        return 2;
    }
    FILE* output_file = fopen("a.obj", "wb");
    if (output_file == NULL) {
        fprintf(stderr, "Error: Cannot create output file.\n");
        fclose(input_file);
        return 2;
    }
    parse_labels(input_file);
    rewind(input_file);

    // Write the initial memory location to load
    uint16_t load_address = htons(DEFAULT_CODESTART);
    fwrite(&load_address, sizeof(load_address), 1, output_file);
    char line[MAX_LINE_LENGTH];
    current_address = DEFAULT_CODESTART;
    while (fgets(line, sizeof(line), input_file) != NULL) {
        char* comment_ptr = strchr(line, '#');
        if (comment_ptr != NULL) {
            *comment_ptr = '\0';
        }
        if (line[0] == '\0' || strchr(line, ':') != NULL) {
            continue;
        }
        char instruction[MAX_LINE_LENGTH];
        char operand[MAX_LINE_LENGTH];
        sscanf(line, "%s %[^\n]", instruction, operand);
        opcode_t opcode;
        uint16_t machine_code;
        uint16_t label_address = 0;
        char dest_reg_str[4];
        char src_reg_str[4];
        char value_str[MAX_LINE_LENGTH];
        reg_t dest_reg;
        reg_t src_reg;
        reg_t base_reg;
        uint16_t value;
        uint16_t offset;
        bool is_imm;

        // Compare instruction with each possible opcode
        if (strcmp(instruction, "add") == 0) {
            sscanf(operand, "%[^, \t\n\r], %[^, \t\n\r], %[^\n]",
                    dest_reg_str, src_reg_str, value_str);
            dest_reg = map_register(dest_reg_str);
            src_reg = map_register(src_reg_str);
            is_imm = (value_str[0] != '%');
            if (is_imm) {
                error_val(value_str, input_file, output_file);
            }
            value = (is_imm) ? atoi(value_str+1) : map_register(value_str);
            if (is_imm) {
                machine_code = emit_add_imm(dest_reg, src_reg, value);
            } else {
                machine_code = emit_add_reg(dest_reg, src_reg, value);
            }
        } else if (strcmp(instruction, "and") == 0) {
            sscanf(operand, "%[^, \t\n\r], %[^, \t\n\r], %[^\n]",
                    dest_reg_str, src_reg_str, value_str);
            dest_reg = map_register(dest_reg_str);
            src_reg = map_register(src_reg_str);
            is_imm = (value_str[0] != '%');
            if (is_imm) {
                error_val(value_str, input_file, output_file);
            }
            value = (is_imm) ? atoi(value_str+1) : map_register(value_str);
            if (is_imm) {
                machine_code = emit_and_imm(dest_reg, src_reg, value);
            } else {
                machine_code = emit_and_reg(dest_reg, src_reg, value);
            }
        } else if (strstr(instruction, "br") != NULL) {
            bool n_flag = false;
            bool z_flag = false;
            bool p_flag = false;
            if (strstr(instruction, "n") != NULL) {
                n_flag = true;
            }
            if (strstr(instruction, "z") != NULL) {
                z_flag = true;
            }
            if (strstr(instruction, "p") != NULL) {
                p_flag = true;
            }
            char label_name[MAX_LINE_LENGTH];
            sscanf(instruction, operand, label_name);
            offset = label_address - current_address;
            machine_code = emit_br(n_flag, z_flag, p_flag, offset);
        } else if (strcmp(instruction, "jmp") == 0) {
            if (strcmp(operand, "ret") == 0) {
                machine_code = emit_jmp(R_R7);
            } else {
                base_reg = map_register(operand);
                machine_code = emit_jmp(base_reg);
            }
        } else if (strcmp(instruction, "jsr") == 0 ||
                    strcmp(instruction, "jsrr") == 0) {
            if (strcmp(instruction, "jsr") == 0) {
                char label_name[MAX_LINE_LENGTH];
                sscanf(operand, "%[^\n]", label_name);
                offset = label_address - current_address+2;
                machine_code = emit_jsr(offset);
            } else if (strcmp(instruction, "jsrr") == 0) {
                base_reg = map_register(operand);
                machine_code = emit_jsrr(base_reg);
            }
        } else if (strcmp(instruction, "ld") == 0) {
            char label_name[MAX_LINE_LENGTH];
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    dest_reg_str, label_name);
            label_address = find_addy(label_name);
            offset = 0;
            dest_reg = map_register(dest_reg_str);
            machine_code = emit_ld(dest_reg, offset);
        } else if (strcmp(instruction, "ldi") == 0) {
            char label_name[MAX_LINE_LENGTH];
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    dest_reg_str, label_name);
            offset = label_address - current_address+2;
            dest_reg = map_register(dest_reg_str);
            machine_code = emit_ldi(dest_reg, offset);
        } else if (strcmp(instruction, "ldr") == 0) {
            sscanf(operand, "%[^, \t\n\r], %[^, \t\n\r], %hd",
                    dest_reg_str, src_reg_str, &offset);
            dest_reg = map_register(dest_reg_str);
            base_reg = map_register(src_reg_str);
            machine_code = emit_ldr(dest_reg, base_reg, offset);
        } else if (strcmp(instruction, "lea") == 0) {
            char label_name[MAX_LINE_LENGTH];
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    dest_reg_str, label_name);
            offset = label_address - current_address+1;
            dest_reg = map_register(dest_reg_str);
            machine_code = emit_lea(dest_reg, offset);
        } else if (strcmp(instruction, "not") == 0) {
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    dest_reg_str, src_reg_str);
            dest_reg = map_register(dest_reg_str);
            src_reg = map_register(src_reg_str);
            machine_code = emit_not(dest_reg, src_reg);
        } else if (strcmp(instruction, "st") == 0) {
            char label_name[MAX_LINE_LENGTH];
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    src_reg_str, label_name);
            src_reg = map_register(src_reg_str);
            offset = label_address - current_address+2;
            machine_code = emit_st(src_reg, offset);
        } else if (strcmp(instruction, "sri") == 0) {
            char label_name[MAX_LINE_LENGTH];
            sscanf(operand, "%[^, \t\n\r], %[^\n]",
                    src_reg_str, label_name);
            src_reg = map_register(src_reg_str);
            offset = label_address - current_address+2;
            machine_code = emit_sti(src_reg, offset);
        } else if (strcmp(instruction, "str") == 0) {
            sscanf(operand, "%[^, \t\n\r], %[^, \t\n\r], %[^\n]",
                    dest_reg_str, src_reg_str, value_str);
            dest_reg = map_register(dest_reg_str);
            src_reg = map_register(src_reg_str);
            offset = atoi(value_str);
            machine_code = emit_str(src_reg, base_reg, offset);
        } else if (strcmp(instruction, "getc") == 0) {
            machine_code = emit_trap(TRAP_GETC);
        } else if (strcmp(instruction, "putc") == 0) {
            machine_code = emit_trap(TRAP_OUT);
        } else if (strcmp(instruction, "puts") == 0) {
            machine_code = emit_trap(TRAP_PUTS);
        } else if (strcmp(instruction, "enter") == 0) {
            machine_code = emit_trap(TRAP_IN);
        } else if (strcmp(instruction, "putsp") == 0) {
            machine_code = emit_trap(TRAP_PUTSP);
        } else if (strcmp(instruction, "halt") == 0) {
            machine_code = emit_trap(TRAP_HALT);
        } else if (strcmp(instruction, "val") == 0) {
            error_val(operand, input_file, output_file);
            operand[0] = '0';
            value = atoi(operand);
            machine_code = emit_value(value);
        }
        if (machine_code != 0) {
            current_address += 2;
            uint16_t network_byte_order = htons(machine_code);
            fwrite(&network_byte_order, sizeof(network_byte_order),
                   1, output_file);
        }
        operand[0] = '\0';
        instruction[0] = '\0';
        machine_code = 0;
    }
    fclose(input_file);
    fclose(output_file);
    return 0;
}
