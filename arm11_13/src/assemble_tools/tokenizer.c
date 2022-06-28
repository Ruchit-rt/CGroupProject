#include "tokenizer.h"

/* declarations */

static char *trim_back(char *str);
static char *trim_front(char *str);

tokens_t *tokenize(char *str) {

    tokens_t *tokens = malloc(sizeof(tokens_t));
    char **token_arr = calloc(MAX_NUM_TOKENS, sizeof(char *));
    halfword_t count = 0;

    /* trim front and back of string */
    // TODO: MIGHT NOT NEED THIS TRIM IN NEW BUFFER 
    str = trim(str);

    int i = 0;
    while (true) {
        char temp[MAX_TOKEN_SIZE * sizeof(char)];

        if (str[i] == '\0') {
            temp[i] = '\0';
            add_token(token_arr, temp, count);
            count++;
            break;
        }

        if (str[i] == ' ' || str[i] == ',') {
            temp[i] = '\0';
            add_token(token_arr, temp, count);
            count++;
            str = trim_front(&str[i]);
            i = 0;
            continue;
        }
        temp[i] = str[i];
        i++;
    }
    
    tokens->array_tokens = token_arr;
    tokens->size = count;
    return tokens;
}

static char *trim_front(char *str) {
    while (*(str) == ' ' || *(str) == ',') {
        str++;
    }
    return str;
}

static char *trim_back(char *str) {
    int pos = strlen(str) - 1;
    while (str[pos] == ' ') {
        pos--;
    }
    /* we found a normal char -- so place \0 after it */
    str[pos + 1] = '\0';
    return str;
}

char *trim(char *str) {
    return trim_back(trim_front(str));
}

void add_token(char **token_arr, char *token, int index) {
    char *to_be_added = malloc(strlen(token) * sizeof(char));
    strcpy(to_be_added, token);
    token_arr[index] = to_be_added;
}

void free_tokens(tokens_t *tokens) {
    for (int i = 0; i < tokens -> size; i++) {
        free(tokens->array_tokens[i]);
    }
    free(tokens->array_tokens);
    free(tokens);
}