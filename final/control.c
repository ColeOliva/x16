#include <stdio.h>
#include <stdlib.h>
#include "bits.h"
#include "control.h"
#include "instruction.h"
#include "x16.h"
#include "trap.h"
#include "decode.h"


// Update condition code based on result
void update_cond(x16_t* machine, reg_t reg) {
    uint16_t result = x16_reg(machine, reg);
    if (result == 0) {
        x16_set(machine, R_COND, FL_ZRO);
    } else if (is_negative(result)) {
        x16_set(machine, R_COND, FL_NEG);
    } else {
        x16_set(machine, R_COND, FL_POS);
    }
}



// Execute a single instruction in the given X16 machine. Update
// memory and registers as required. PC is advanced as appropriate.
// Return 0 on success, or -1 if an error or HALT is encountered.
int execute_instruction(x16_t* machine) {
    // Fetch the instruction and advance the program counter
    uint16_t pc = x16_pc(machine);
    uint16_t instruction = x16_memread(machine, pc);
    x16_set(machine, R_PC, pc + 1);

    if (LOG) {
        fprintf(LOGFP, "0x%x: %s\n", pc, decode(instruction));
    }

    // Variables we might need in various instructions
    reg_t dst, src1, src2, base;
    uint16_t result, indirect, offset, imm, cond, jsrflag, op1, op2;

    // Decode the instruction
    uint16_t opcode = getopcode(instruction);
    switch (opcode) {
        case OP_ADD:
            imm = getbit(instruction, 5);
            src1 = getbits(instruction, 6, 3);
            op1 = x16_reg(machine, src1);
            if (imm == 0) {
                src2 = getbits(instruction, 0, 3);
                op2 = x16_reg(machine, src2);
            } else {
                op2 = sign_extend(getbits(instruction, 0, 5), 5);
            }
            dst = getbits(instruction, 9, 3);
            result = op1 + op2;

            // Update destination register and condition code
            x16_set(machine, dst, result);
            update_cond(machine, dst);
            break;

        case OP_AND:
            src1 = getbits(instruction, 6, 3);
            dst = getbits(instruction, 9, 3);
            imm = getbit(instruction, 5);

            op1 = x16_reg(machine, src1);
            if (imm) {
                op2 = sign_extend(getbits(instruction, 0, 5), 5);
            } else {
                src2 = getbits(instruction, 0, 3);
                op2 = x16_reg(machine, src2);
            }
            result = op1 & op2;

            x16_set(machine, dst, result);
            if (is_negative(result)) {
                x16_set(machine, R_COND, FL_NEG);
            } else if (result == 0) {
                x16_set(machine, R_COND, FL_ZRO);
            } else {
                x16_set(machine, R_COND, FL_POS);
            }
            break;

        case OP_NOT:
            src1 = getbits(instruction, 6, 3);
            dst = getbits(instruction, 9, 3);

            // Get the value from the source register, compute complement
            op1 = x16_reg(machine, src1);
            result = ~op1;

            // Store the result in the destination register
            x16_set(machine, dst, result);

            // Update condition codes based on the result
            update_cond(machine, dst);
            break;

        case OP_BR:
            uint16_t n_flag = getbit(instruction, 11);
            uint16_t z_flag = getbit(instruction, 10);
            uint16_t p_flag = getbit(instruction, 9);
            offset = sign_extend(getbits(instruction, 0, 9), 9);

            // Get the current program counter
            uint16_t pc = x16_pc(machine);

            // Check any of the conditions
            if ((n_flag && x16_cond(machine) == FL_NEG) ||
                (z_flag && x16_cond(machine) == FL_ZRO) ||
                (p_flag && x16_cond(machine) == FL_POS) ||
                (!z_flag && !n_flag && !p_flag)) {
                // Branch to the specified location
                x16_set(machine, R_PC, pc + offset);
            }
            break;

        case OP_JMP:
            base = getbits(instruction, 6, 3);

            if (base == R_R7) {
                x16_set(machine, R_PC, x16_reg(machine, R_R7));
            } else {
                x16_set(machine, R_PC, x16_reg(machine, base));
            }
            break;

        case OP_JSR:
            x16_set(machine, R_R7, x16_pc(machine));

            if (getbit(instruction, 11) == 0) {
                // Subroutine address obtained from base register
                base = getbits(instruction, 6, 3);
                x16_set(machine, R_PC, x16_reg(machine, base));
            } else {
                // Compute subroutine address
                offset = sign_extend(getbits(instruction, 0, 11), 11);
                x16_set(machine, R_PC, x16_pc(machine) + offset);
            }
            break;

        case OP_LD:
            offset = sign_extend(getbits(instruction, 0, 9), 9);
            uint16_t address = x16_pc(machine) + offset;
            result = x16_memread(machine, address);
            dst = getbits(instruction, 9, 3);
            x16_set(machine, dst, result);
            update_cond(machine, dst);
            break;

        case OP_LDI:
            offset = sign_extend(getbits(instruction, 0, 9), 9);
            address = x16_pc(machine) + offset;
            uint16_t data_addy = x16_memread(machine, address);
            result = x16_memread(machine, data_addy);
            dst = getbits(instruction, 9, 3);
            x16_set(machine, dst, result);
            update_cond(machine, dst);
            break;

        case OP_LDR:
            offset = sign_extend(getbits(instruction, 0, 6), 6);
            base = getbits(instruction, 6, 3);
            uint16_t base_address = x16_reg(machine, base);
            address = base_address + offset;
            result = x16_memread(machine, address);
            dst = getbits(instruction, 9, 3);
            x16_set(machine, dst, result);
            update_cond(machine, dst);
            break;

        case OP_LEA:
            offset = sign_extend(getbits(instruction, 0, 9), 9);
            pc = x16_pc(machine);
            result = pc + offset;
            dst = getbits(instruction, 9, 3);
            x16_set(machine, dst, result);
            update_cond(machine, dst);
            break;

        case OP_ST:
            src1 = getbits(instruction, 9, 3);
            address = x16_pc(machine) +
                      sign_extend(getbits(instruction, 0, 9), 9);
            op1 = x16_reg(machine, src1);
            x16_memwrite(machine, address, op1);
            break;

        case OP_STI:
            src1 = getbits(instruction, 9, 3);
            address = x16_pc(machine) +
                      sign_extend(getbits(instruction, 0, 9), 9);
            dst = x16_memread(machine, address);
            uint16_t op1 = x16_reg(machine, src1);
            x16_memwrite(machine, dst, op1);
            break;

        case OP_STR:
            src1 = getbits(instruction, 9, 3);
            base = getbits(instruction, 6, 3);
            address = x16_reg(machine, base) +
                      sign_extend(getbits(instruction, 0, 6), 6);
            op1 = x16_reg(machine, src1);
            x16_memwrite(machine, address, op1);
            break;

        case OP_TRAP:
            // Execute the trap -- do not rewrite
            return trap(machine, instruction);

        case OP_RES:
        case OP_RTI:
        default:
            // Bad codes, never used
            abort();
    }

    return 0;
}
