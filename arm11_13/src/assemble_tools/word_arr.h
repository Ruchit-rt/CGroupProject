#ifndef WORD_ARR_H
#define WORD_ARR_H
#include <stdio.h>
#include <stdlib.h>
#include "../defns.h"

/* Words Struct to save the extra words of 32 bits from an ldr instruction 
 * to add to the end of the binary file. Along with that are present, the 
 * capacity of the word array and the current number of elements.
 */
typedef struct
{
    halfword_t capacity;
    word_t *arr;
    halfword_t size;
} word_arr_t;

word_arr_t *make_word_arr(void);
void add_word_arr(word_arr_t *word_arr, word_t word);
void free_word_arr(word_arr_t *word_arr);

#endif
