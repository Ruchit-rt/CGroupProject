#include <stdio.h>
#include <stdlib.h>
#include "execute.h"

/* A N (non-existant) instruction. */
static const inst_t NULL_INSTRUCTION = {
    .type = NUL,
    .cond = 0,
    .shift_amount = 0,
    .i = false,
    .p = false,
    .ua = false,
    .sl = false,
    .rm = 0,
    .rd = 0,
    .rs = 0,
    .rn = 0,
};

static int condition(state_sys_t *state) {
    /* Fetching first 4 bits */
    byte_t flags = state->registers[CPSR] >> (SIZE - 4);

    /* Checking if the condition required by the current decoded
     * instruction is met by the current state of the flags (CPSR) register */
    switch (state->decoded_inst->cond) {
    case EQ:
        return flags & Z;
    case NE:
        return !(flags & Z);
    case GE:
        return (flags & V) == ((flags & N) >> 3);
    case LT:
        return (flags & V) != ((flags & N) >> 3);
    case GT:
        return !(flags & Z) && ((flags & V) == ((flags & N) >> 3));
    case LE:
        return (flags & Z) || ((flags & V) != ((flags & N) >> 3));
    case AL:
        return 1;   
    default:
        fprintf(stderr, "Illegal COND flag");
        return -1;
    }
}

static word_t negate(word_t val) {
    return (~val + 1);
}

static bool is_neg(word_t value) {
    return value >> 31;
}

static long absolute(word_t value) { 
    if (is_neg(value)) {
        return (long) negate(value);
    } 
    return (long) value;
}

static long offset_in_long(word_t value) {
    long result = absolute(value);
    if (is_neg(value)) {
        result *= -1;
    }
    return result;
}

/* One execute cycle is run. */

int execute(state_sys_t *state) {
    if (condition(state)) {
        switch (state->decoded_inst->type) {
            case DP:
            return execute_dp(state);
            case MUL:
            return execute_multiply(state);
            case SDT:
            return execute_sdt(state);
            break;
            case BRA:
            return execute_branch(state);
            break;
            default:
            fprintf(stderr, "Illegal instruction type");
            return -1;
        }
    }
    return 0;
}

carryval_t *shifter(shift_t type, word_t shift_amount, word_t value) {
    carryval_t *res = malloc(sizeof(carryval_t));

    if (!res) {
        printf("Unable to allocate memory for result of shifter");
    }

    switch(type) {
        case LSL:
            res->value = (shift_amount >= SIZE) ? 0 : value << shift_amount;
            res->carry = (value >> (SIZE - shift_amount)) & 0x1;
            break;
        case LSR:
            res->value = (shift_amount >= SIZE) ? 0 : value >> shift_amount;
            res->carry = (value << (SIZE - shift_amount)) & 0x80000000;
            break;
        case ASR:
            res->value = (value >> shift_amount)
                        | ((value & 0x80000000) ?
                            ~((1L << (SIZE - shift_amount)) - 1L) : 0L);
            res->carry = (value << (SIZE - shift_amount)) & 0x80000000;
            break;
        case ROR:
            res->value = (value << (SIZE - shift_amount))
                        | (value >> shift_amount);
            res->carry = (value << (SIZE - shift_amount)) & 0x80000000;
            break;
        default:
            fprintf(stderr, "Unknown shift type");
            exit(EXIT_FAILURE);
    }

    if (shift_amount == 0) {
        res->carry = false;
    }

    return res;
}

/* Executes DATA PROCESSING (DP) instruction. */

int execute_dp(state_sys_t *state) {
    word_t shift_amount;
    word_t op2;
    inst_t *inst = state->decoded_inst;
    bool sh_carry = 0;
    
    if (inst->i) {
        op2 = inst->offset;
        shift_amount = inst->shift_amount;
        // printf("shift amount:0x%x\n", shift_amount);
    } 
    else {
        op2 = state->registers[inst->rm];
        if (inst->rs == 0) {
            shift_amount = inst->shift_amount;
        } 
        else {
            shift_amount = state->registers[inst->rs];
        }
    }

    carryval_t *shiftop2 = shifter(inst->shift, shift_amount, op2);
    sh_carry = shiftop2->carry;
    op2 = shiftop2->value;
    free(shiftop2);
    word_t flags = 0;
    word_t res;

    switch (inst->op) {
    case EOR:
    case TEQ:
        res = state->registers[inst->rn] ^ op2;
        flags = C * sh_carry;
        break;
    case AND:
    case TST:
        res = state->registers[inst->rn] & op2;
        flags = C * sh_carry;
        break;
    case RSB:
        res = op2 + negate(state->registers[inst->rn]);
        flags = C * ((is_neg(state->registers[inst->rn])
            == is_neg(op2)) !=  is_neg(res));
        break;
    case SUB:
    case CMP:
        res = state->registers[inst->rn] + negate(op2);
        flags = C * ((is_neg(state->registers[inst->rn])
            == is_neg(op2)) !=  is_neg(res));
        break;
    case ORR:
        res = state->registers[inst->rn] | op2;
        flags = C * sh_carry;
        break;
    case ADD:
        res = state->registers[inst->rn] + op2;
        flags = C * ((is_neg(state->registers[inst->rn])
            == is_neg(op2)) !=  is_neg(res));
        break;
    case MOV:
        res = op2;
        flags = C * sh_carry;
        break;
    default:
        res = 0;
        perror("Incorrect op type in DP\n");
        return -1;
    }

    flags |= (N * is_neg(res));
    flags |= (Z * (res == 0));

    if (!(inst->op == TST || inst->op == CMP || inst->op == TEQ)) {
        state->registers[inst->rd] = res;
    }

    if (inst->sl) {
        state->registers[CPSR] &= MASK_4;
        state->registers[CPSR] |= (flags << (SIZE - 4));
    }
    return 1;
}

/* Executes MULTIPLY (MUL) instruction. */

int execute_multiply(state_sys_t *state) {
    inst_t *inst = state->decoded_inst;
    int rdi = inst->rd;
    int rsi = inst->rs;
    int rmi = inst->rm;
    int rni = inst->rn;
    word_t res = state->registers[rmi] * state->registers[rsi];
    if (inst->ua > 0) {
        res += state->registers[rni];
    } 
    state->registers[rdi] = res;
    if (inst->sl > 0) {
        if (res == 0) {
            state->registers[CPSR] |= 0x4000000;
        }
        if (res >> 31) {
            state->registers[CPSR] |= 0x80000000;
        } 
        else {
            state->registers[CPSR] &= 0x7FFFFFFF;
        }
    }
    return 1;
}

static bool in_GPIO(word_t address) {
    if (address >= GPIO_START_ADDR && address < GPIO_END_ADDR) {
        printf("One GPIO pin from %u to %u has been accessed\n",
           (address - GPIO_START_ADDR) / 4 * 10,
           (address - GPIO_START_ADDR) / 4 * 10 + 9);
           return true;
    } else if (address >= GPIO_CLEAR_START
    && address < GPIO_CLEAR_START + GPIO_CLEAR_SIZE) {
        // GPIO pin cleared
        printf("PIN OFF\n");
        return true;
  } else if (address >= GPIO_SET_START
    && address < GPIO_SET_START + GPIO_SET_SIZE) {
    // GPIO pin set
    printf("PIN ON\n");
    return true;
  }
    return false;
}

/* Executes SINGLE DATA TRANSFER (SDT) instruction. */

int execute_sdt(state_sys_t *state) {
    word_t shift_amount;
    word_t offset; 
    word_t address;
    inst_t *instruct = state->decoded_inst;

    if(instruct->i){
        if(instruct->rs == 0){
            shift_amount = instruct->shift_amount;
        } 
        else{
            shift_amount = state->registers[instruct->rs];
        }
        carryval_t *shift_out = shifter(instruct->shift, shift_amount, state->registers[instruct->rm]);
        offset = shift_out->value;
        free(shift_out);                         
    } 
    else {
        offset = instruct->offset;
    }

    if (!instruct->ua) {
        offset = negate(offset);
    }

    if(instruct->p) {
        address = state->registers[instruct->rn] + offset;
    }

    else {
        address = state->registers[instruct->rn];
        state->registers[instruct->rn] = address + offset;
    }

    if(instruct->sl) {
        /* Load instruction */
        byte_t *memory = state->memory;
        word_t res = 0;
        
        if (address >= MAX_BYTES - 4) {
            if (in_GPIO(address)) {
                /* GPIO PIN ACCESS TO LOAD */
                res = address;
            } 
            else {
                printf("Error: Out of bounds memory access at address 0x%08x\n", address);
                res = 0;
            }
        } 
        else {
            for (int i = 3; i >= 0; i--) {
                res = (res << 8) | *(memory + address + i);
            }
        }
        state->registers[instruct->rd] = res;
    }
    else {
        /* Store instruction */
        if (address >= MAX_BYTES - 4) {
            if (in_GPIO(address)) {
                /* GPIO PIN ACCESS TO LOAD */
                return 0;
            } 
            else {
                printf("Error: Out of bounds memory access at address 0x%08x\n", address);
                return 0;
            }
        } 
        else {
            word_t word= state->registers[instruct->rd];
            for(int j = 0; j < 4; j++){
                state->memory[address+j] = (byte_t) (word & 0xFF);
                word = word >> 8;
            }
        }
    }
    return 1;
}

/* Executes BRANCH (BRA) instruction. */

int execute_branch(state_sys_t *state) {
    state->registers[PC] += offset_in_long(state->decoded_inst->branch_offset);
    state->has_fetched = false;
    *(state->decoded_inst) = NULL_INSTRUCTION;
    return 1;
}