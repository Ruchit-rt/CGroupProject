#include "symbol_table.h"

/* first pass returning a new file buffer, symbol table, and new line num */
new_buffer *generate_symbol_table(char **file_buffer, int num_lines) {
    label_map *lmap = malloc(sizeof(label_map));
    label_t **label_arr = malloc(num_lines * sizeof(label_t *));
    word_t addr = 0;
    new_buffer *new_buff = malloc(sizeof(new_buffer));
    char **new_file_buff = malloc(num_lines * sizeof(char *));
    int new_line_num = 0;
    int count_label = 0;
    char *str;
    for (int i = 0; i < num_lines; i++) {
        bool is_label = false;
        str = file_buffer[i];
        str = trim(str); 
        for (int i = 0; i < strlen(str); i++) {
            if (str[i] == ':') {
                is_label = true;
                break; 
            }
    }

    /* check if is_label */
    if (is_label) {
        /* got a label */
        label_t *label_row = malloc(sizeof(label_t));
        strcpy(label_row ->label, str);
        label_row->address = addr;
        label_arr[count_label] = label_row;
        count_label++;
    } else {
        /* it is a normal instruction */
        addr += 4;
        char *to_be_added = malloc(strlen(file_buffer[i]));
        strcpy(to_be_added, str);
        new_file_buff[new_line_num] = to_be_added;
        new_line_num++;
    }
  }

  lmap->labels = label_arr;
  lmap->size = count_label;

  new_buff->map = lmap;
  new_buff->new_file_buffer = new_file_buff;
  new_buff->new_num_lines = new_line_num;

  return new_buff;
}

/* lookup address corresponding to a label */
word_t lookup(label_map *map, char *label_name) {
    for (int i = 0; i < map->size; i++) { 
        if (!strcmp((map->labels[i])->label, label_name)) {
            return (map->labels[i])->address;
        }
    }
    perror("Cant find label in table \n");
    return -1;
}

/* free a 2 dimensional spine array with given number of rows */
void free_spine_array(char ** file_buffer, int rows) {
  for (int i = 0; i < rows; i++) {
    free(file_buffer[i]);
  }
  free(file_buffer);
}

/* free map recursively going into the label rows */
void free_map(label_map *map) {
    for (int i = 0; i < map->size; i++) {
        free((map->labels)[i]);
    }
    free(map->labels);
    free(map);
}

/* free the new buffer and all its components */
void free_buffer(new_buffer *new_buff) {
    free_map(new_buff->map);
    free_spine_array(new_buff->new_file_buffer, new_buff->new_num_lines);
    free(new_buff);
}