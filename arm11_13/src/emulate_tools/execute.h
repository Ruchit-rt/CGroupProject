#ifndef EXECUTE_H
#define EXECUTE_H
#include "state_of_sys.h"
#include "../defns.h"
#include "../instruction.h"

int execute(state_sys_t *state);
int execute_dp(state_sys_t *state);
int execute_multiply(state_sys_t *state);
int execute_sdt(state_sys_t *state);
int execute_branch(state_sys_t *state);

/* struct for barrel shifter carry value */
typedef struct {
    word_t value;
    bool carry;
} carryval_t;

#endif