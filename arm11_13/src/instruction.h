#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdbool.h>
#include "defns.h"

//SET OF TYPES OF INSTRUCTIONS
typedef enum {
    DP,
    MUL,
    SDT,
    BRA,
    HAL,
    NUL,
    SPL
} inst_type_t;

//SET OF OPCODES
typedef enum {
    AND = 0x0,
    EOR = 0x1,
    SUB = 0x2,
    RSB = 0x3,
    ADD = 0x4,
    TST = 0x8,
    TEQ = 0x9,
    CMP = 0xA,
    ORR = 0xC,
    MOV = 0xD,
} opcode_t;

//SET OF COND CODES
typedef enum {
    EQ = 0x0,
    NE = 0x1,
    GE = 0xA,
    LT = 0xB,
    GT = 0xC,
    LE = 0xD,
    AL = 0xE,
} cond_t;

//SET OF SHIFT TYPES
typedef enum {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3,
} shift_t;

//CPSR FLAGS
typedef enum {
    N = 0x8,
    Z = 0x4,
    C = 0x2,
    V = 0x1,
} cpsr_t;


typedef struct {
    //Instruction Type
    inst_type_t type;

    //Condition field
    byte_t cond;
    //Opcode
    opcode_t op;
    //Shift-type
    shift_t shift;
    //Shift-amount
    byte_t shift_amount;
    //Operand 2
    halfword_t op2;
    //Offset for SDT and DP - Type Instructions
    halfword_t offset;
    //Offset for Branch
    word_t branch_offset;

    //4-bit registers
    byte_t rn;
    byte_t rd;
    byte_t rs;
    byte_t rm;

    //Flags to store Single bit attributes
    bool i;
    bool p;
    bool ua;
    bool sl;    
} inst_t;

#endif