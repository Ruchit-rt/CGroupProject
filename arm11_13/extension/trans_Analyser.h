#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LEN 512
#define READ 0
#define WRITE 1
#define COMMIT 2
#define ABORT 3
#define MAX_DIGIT_SIZE 10
#define MAX_NAME_LEN 10

typedef uint8_t byte_t;

typedef struct item
{
    byte_t op;
    byte_t num;
    char name[MAX_NAME_LEN];
} item;

typedef struct conflict 
{
    int first;
    int second;
} conflict;

int parse_line(item **history_arr, char *line, int index);
void update_names(char **object_name, char* name);
int sum_array(int *num_items, int len);
bool same_obj(item *first, item* second);
bool is_conflict(item *first, item* second);
bool same_name(item element, char *name);
int nc2(int num);
bool compare_conflicts(conflict first, conflict second);
void get_set(conflict** conflicts_set, conflict** conflicts);
bool find_cycle(int root, bool visited[], int parent[], int len, conflict **conflict_set);
void free_spine(void **arr, int len);
int get_write(item object, item* target, int index);
bool check_commit_order(item write, item read, item *target);
bool is_dirty(int write_index, int read_index, item *target);