#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"

typedef struct {
    char label[MAX_LABEL_LEN];
    word_t address;
} label_t;

typedef struct {
    halfword_t size;
    label_t **labels;
} label_map;

typedef struct {
    char **new_file_buffer;
    int new_num_lines;
    label_map* map;
} new_buffer;

extern new_buffer *generate_symbol_table(char **file_buffer, int num_lines);
extern void free_spine_array(char ** file_buffer, int rows);
extern void free_map(label_map *map);
extern void free_buffer(new_buffer *new_buff);
extern word_t lookup(label_map *map, char *label_name);

#endif