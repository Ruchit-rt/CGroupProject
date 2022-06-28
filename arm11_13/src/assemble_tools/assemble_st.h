#ifndef ASSEMBLE_ST_H
#define ASSEMBLE_ST_H
#include "../defns.h"
#include "../instruction.h"

//SET OF MNEMONIC CODES
typedef enum {
    ANDm,
    EORm,
    SUBm,
    RSBm,
    ADDm,
    TSTm,
    TEQm,
    CMPm,
    ORRm,
    MOVm,
    MULm,
    MLAm,
    LDRm,
    STRm,
    BEQm,
    BNEm,
    BGEm,
    BLTm,
    BGTm,
    BLEm,
    Bm,
    LSLm,
    ANDEQm,
} mne_t;

//ASSEMBLER STRUCT
typedef struct {
    //Mnemonic type
    mne_t mn_type;
    //Instruction type
    inst_type_t inst_type;

    //Condition field
    byte_t cond;
    //Opcode
    opcode_t op;
    //Operand 2
    halfword_t op2;
    //Offset for SDT and DP - Type Instructions
    halfword_t offset;
    //Offset for Branch Type Instruction
    word_t branch_offset;

    //Flags to store Single bit attributes
    bool i;
    bool p;
    bool ua;
    bool sl;

    //4-bit registers
    byte_t rn;
    byte_t rd;
    byte_t rs;
    byte_t rm;   
} ass_t;

//Tokens Struct to save the tokens and number of tokens in the array
typedef struct
{
    char **array_tokens;
    halfword_t size;
} tokens_t;

#endif