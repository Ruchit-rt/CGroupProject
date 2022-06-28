#ifndef DECODE_H
#define DECODE_H

#include "state_of_sys.h"
#include "../defns.h"
#include "../instruction.h"

void decode(state_sys_t *state);
void halt(state_sys_t *state);
void dp(state_sys_t *state);
void multiply(state_sys_t *state);
void sdt(state_sys_t *state);
void branch(state_sys_t *state);

#endif