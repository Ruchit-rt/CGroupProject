#ifndef STATE_OF_SYS_H
#define STATE_OF_SYS_H

#include "../defns.h"
#include "../instruction.h"

typedef struct {
    word_t registers[NUM_OF_REGS];
    byte_t memory[NUM_OF_ADDRS];
    word_t fetched_inst; 
    bool has_fetched;
    inst_t *decoded_inst;
} state_sys_t;

#endif