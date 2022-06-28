#ifndef DEFNS_H
#define DEFNS_H
#include <stdint.h>

typedef uint32_t word_t;
typedef uint8_t byte_t;
typedef uint16_t halfword_t;
#define MAX_BYTES 65536
#define PC 15
#define CPSR 16
#define SIZE 32
#define MASK_4 0xFFFFFFF
#define MASK_8 0xFFFFFF
#define NUM_OF_REGS 17
#define NUM_OF_ADDRS 65536
#define GPIO_START_ADDR 0x20200000
#define GPIO_END_ADDR 0x2020000C
#define GPIO_CLEAR_START 0x20200028
#define GPIO_CLEAR_SIZE 4
#define GPIO_SET_START 0x2020001C
#define GPIO_SET_SIZE 4
#define max_char_inline 512
#define MAX_TOKEN_SIZE 16
#define MAX_NUM_TOKENS 6
#define MAX_LABEL_LEN 64
#define PIPELINE_OFFSET 8

/* MACRO DEFINITION for enum return funtion */
#define GET_ENUM(op, enum)\
if (!(strcmp(str, op))) {\
    return enum;\
  }

/* MACRO DEFINITION FOR SHIFT TYPES */
#define GET_SHIFT(str, shift_type)\
if (!strcmp(shift_str, str)) {\
        shift = shift_type;\
    }

#endif