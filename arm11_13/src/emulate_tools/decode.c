#include <stdlib.h>
#include <stdio.h>
#include "decode.h"

void decode(state_sys_t *state) {
    word_t fetched = state->fetched_inst;
    inst_t *inst = state->decoded_inst;
    inst->cond = fetched >> (SIZE - 4);
    
    fetched &= MASK_4;

    if(!fetched) {
        /* HALT instruction */
        halt(state);
    }
    else if((fetched >> (SIZE - 8)) == 0xA) {
        /* BRANCH instruction */
        branch(state);
    }
    else if(!(fetched >> 22) && (((fetched >> 4) & 0xF) == 0x9)) {
        /* MUL instruction */
        multiply(state);
    }
    else if(!(fetched >> (SIZE - 6))) {
        /* DATA PROCESSING instruction */
        dp(state);
    }
    else if((fetched >> (SIZE - 6)) == 0x1) {
        /* SINGLE DATA TRANSFER instruction */
        sdt(state);
    }
    else {
        fprintf(stderr, "Unknown instruction\n");
    }
}

/* Decodes HALT (HAL) instruction. */

void halt(state_sys_t *state) {
    state->decoded_inst->type = HAL;
}

/* Decodes DATA PROCESSING (DP) instruction. */

void dp(state_sys_t *state) {
    word_t fetched = state->fetched_inst;
    inst_t *inst = state->decoded_inst;
    inst->type = DP;
    inst->i = (fetched >> 25) & 0x1;
    inst->op = (fetched >> 21) & 0xF;
    inst->sl = (fetched >> 20) & 0x1;
    inst->rn = (fetched >> 16) & 0xF;
    inst->rd = (fetched >> 12) & 0xF;
    inst->op2 = fetched & 0xFFF;

    if(inst->i) {
        inst->offset = fetched & 0xFF;
        inst->shift = ROR;
        inst->shift_amount = ((fetched >> 8) & 0xF) << 1;
    }
    else {
        inst->rm = fetched & 0xF;
        inst->shift = (fetched >> 5) & 0x3;
        
        if((fetched >> 4) & 0x1) {
            inst->rs = (fetched >> 8) & 0xF;            
        }
        else {
            inst->shift_amount = (fetched >> 7) & 0x1F;
        }
    }
}

/* Decodes MULTIPLY (MUL) instruction. */

void multiply(state_sys_t *state) {
    word_t fetched = state->fetched_inst;
    inst_t *inst = state->decoded_inst;
    inst->type = MUL;
    inst->ua = (fetched >> 21) & 0x1;
    inst->sl = (fetched >> 20) & 0x1;
    inst->rm = fetched & 0xF;
    inst->rs = (fetched >> 8) & 0xF;
    inst->rn = (fetched >> 12) & 0xF;
    inst->rd = (fetched >> 16) & 0xF;
}

/* Decodes SINGLE DATA TRANSFER (SDT) instruction. */

void sdt(state_sys_t *state) {
    word_t fetched = state->fetched_inst;
    inst_t *inst = state->decoded_inst;
    inst->type = SDT;
    inst->i = (fetched >> 25) & 0x1;
    inst->p = (fetched >> 24) & 0x1;
    inst->ua = (fetched >> 23) & 0x1;
    inst->sl = (fetched >> 20) & 0x1;
    inst->rn = (fetched >> 16) & 0xF;
    inst->rd = (fetched >> 12) & 0xF;

    if(inst->i) {
        inst->rm = fetched & 0xF;
        inst->shift = (fetched >> 5) & 0x3;
        
        if((fetched >> 4) & 0x1) {
            inst->rs = (fetched >> 8) & 0xF;            
        }
        else {
            inst->shift_amount = (fetched >> 7) & 0x1F;
        }
    } 
    else {
        inst->offset = fetched & 0xFFF;
    }
}

/* Decodes BRANCH (BRA) instruction. */

void branch(state_sys_t *state) {
    state->decoded_inst->type = BRA;
    uint32_t offset = state->fetched_inst & MASK_8;
    offset = offset << 2;

    /* sign extending */
    if((state -> fetched_inst >> 23) & 0x1) {
        offset |= 0xFC000000;
    }
    state->decoded_inst->branch_offset = offset; 
}