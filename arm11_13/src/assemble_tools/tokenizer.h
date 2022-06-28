#ifndef TOKENIZER_H

#define TOKENIZER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../defns.h"
#include "assemble_st.h"

extern tokens_t *tokenize(char *str);
extern char *trim(char * str);
extern void free_tokens(tokens_t *tokens);
extern void add_token(char **token_arr, char *token, int index);

#endif

