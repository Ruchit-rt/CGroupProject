#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "assemble_st.h"
#include "word_arr.h"
#include "symbol_table.h"

extern void dp_parser(ass_t *assem, tokens_t *tokens);
extern void branch_parser(ass_t *assem, tokens_t *tokens, label_map *map, word_t addr);
extern void mul_parser(ass_t *assem, tokens_t *tokens);
extern void sdt_parser(ass_t *assem, tokens_t *tokens, word_arr_t *more_words, int curr_line, int total_lines);
extern void special_parser(ass_t *assem, tokens_t *tokens);
extern void branch_parser(ass_t *assem, tokens_t *tokens, label_map *map, word_t addr);

#endif