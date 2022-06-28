#include "assembler.h"
#include <stdio.h>

//Reads respective bits from assmbler struct attributes and sets them for DATA PROCESSING Type Instruction
word_t dp_assembler(ass_t *assem) {
    word_t word = assem->cond;
    word <<= 2;
    word = word << 1 | assem->i;
    word = word << 4 | assem->op;
    word = word << 1 | assem->sl;
    word = word << 4 | assem->rn;
    word = word << 4 | assem->rd;    
    word = word << 12 | assem->op2;

    return word;    
}

//Reads respective bits from assmbler struct attributes and sets them for MULTIPLY Type Instruction
word_t mul_assembler(ass_t *assem) {
    word_t word = assem->cond;
    word <<= 6;
    word = word << 1 | assem->ua;
    word = word << 1 | assem->sl;
    word = word << 4 | assem->rd;
    word = word << 4 | assem->rn;
    word = word << 4 | assem->rs;
    word = word << 4 | 0x9;
    word = word << 4 | assem->rm;

    return word;    
} 

//Reads respective bits from assmbler struct attributes and sets them for SINGLE DATA TRANSFER Type Instruction
word_t sdt_assembler(ass_t *assem) {
    word_t word = assem->cond;
    word = word << 2 | 0x1;
    word = word << 1 | assem->i;
    word = word << 1 | assem->p;
    word = word << 1 | assem->ua;
    word <<= 2;
    word = word << 1 | assem->sl;
    word = word << 4 | assem->rn;
    word = word << 4 | assem->rd;
    word = word << 12 | assem->offset;

    return word;    
}

//Reads respective bits from assmbler struct attributes and sets them for BRANCH Type Instruction
word_t branch_assembler(ass_t *assem) {
    word_t word = assem->cond;
    word = word << 4 | 0xA;
    word = word << 24 | assem->branch_offset;

    return word;    
}