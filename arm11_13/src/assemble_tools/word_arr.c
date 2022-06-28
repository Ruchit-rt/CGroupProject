#include "word_arr.h"

// Creates an array to save the extra words of 32 bits from an ldr instruction
word_arr_t *make_word_arr(void) {
    word_arr_t *word_arr = malloc(sizeof(word_arr_t));
    if (!word_arr) {
        fprintf(stderr, "Unable to allocate memory");
    }
    word_arr->capacity = 64;
    word_arr->arr = malloc(word_arr->capacity * sizeof(word_t));
    if (!word_arr->arr) {
        fprintf(stderr, "Unable to allocate memory");
    }
    word_arr->size = 0;
    return word_arr;
}

/* Appends 32-bit words to the Word array and if capacity is exceeded,
 * a new array of double the capacity is allocated */

void add_word_arr(word_arr_t *word_arr, word_t word) {
    if (word_arr->size >= word_arr->capacity) {
        word_arr->capacity *= 2;
        word_arr->arr = realloc(word_arr->arr, word_arr->capacity * sizeof(word_t));
        if (!word_arr->arr) {
            fprintf(stderr, "Unable to increase size");
        }
    }
    word_arr->arr[word_arr->size] = word;
    word_arr->size++;
}

// Frees the Word array after use
void free_word_arr(word_arr_t *word_arr) {
    free(word_arr->arr);
    free(word_arr);
}