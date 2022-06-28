#include "trans_Analyser.h"

int main(int argc, char **argv) {
    assert(argc == 2);
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Cant open file for reading \n");
        exit(EXIT_FAILURE);
    }
    
    /* first line contains number of histories */
    int number_hist;
    fscanf(file ,"%d\n", &number_hist);

    /* creating spine array for all the histories (start from index 0) */
    item **history_arr = malloc((number_hist + 1) * sizeof(item *));

    /* count of number of items in different histories (start from index 1 - target num not stored)*/
    int *num_items = malloc((number_hist + 1) * sizeof(int));

    char line[MAX_LINE_LEN];
    for(int i = 1; i <= number_hist; i++) {
        fgets(line, MAX_LINE_LEN, file);
        
        /* sanitary line check on syntax */
        assert(line[0] == 'H');
        char str_num[MAX_DIGIT_SIZE];
        for (int i = 1; line[i] != ':'; i++) {
            str_num[i - 1] = line[i];
        }
        assert(atoi(str_num) == i);
        assert(line[2] == ':');
        num_items[i] = parse_line(history_arr, &line[3], i - 1);
    }

    /* we parse the target history */
    fgets(line, MAX_LINE_LEN, file);
    int i = 0;
    for (; line[i] != ':'; i++);
    i++;
    int num_target = parse_line(history_arr, &line[i], number_hist);

    /* sanitary check on the number of tokens */
    if (num_target > sum_array(num_items, number_hist)) {
        perror("Target does not have correct number of tokens\n");
        exit(EXIT_FAILURE);
    }

    /* intialise object names array */
    char **object_names = malloc(num_target * sizeof(char *));
    if (!object_names) {
        perror("could not allocate object names array \n");
        exit(EXIT_FAILURE);
    }

    /* build object names list in second pass */
    for(int i = 0; i < number_hist; i++) {
        item *history = history_arr[i];
        for (int j = 0; j < num_items[i + 1]; j++) {
            if (history[j].name[0] != '\0') {
                update_names(object_names, history[j].name);
            }
        }
    }

    /* create an array for the confilcts (indeced from 0) */
    int max_conflicts = nc2(num_target);
    conflict **conflicts = malloc(max_conflicts * sizeof(conflict *));
    int index = 0;

    /* go through target and build the conflicts */
    item *target = history_arr[number_hist];
    for (int i = 0; i < num_target; i++) {
        item curr = target[i];
        for (int j = i + 1; j < num_target; j++) {
            if (is_conflict(&curr, &target[j])) {
                conflict *confl = malloc(sizeof(conflict));
                confl->first = curr.num;
                confl->second = target[j].num;
                conflicts[index] = confl;
                index++;
                break;
            } 
            if (same_obj(&curr, &target[j]) && curr.num == target[j].num) {
                if (curr.op == 0 && target[j].op == 1) {
                    break;
                } 
            }
        }
    }

    conflict **conflicts_set = malloc(max_conflicts * sizeof(conflict *));

    /* get unique conflicts */
    get_set(conflicts_set, conflicts);

    /* Serialisability Analysis - recursively dfs through the conflicts */
    int *parent = malloc((number_hist + 1) * sizeof(int));
    bool *visited = malloc((number_hist + 1) * sizeof(bool));
    
    bool serilisable;
    int root;
    if (conflicts_set[0]) {
        root = conflicts_set[0]->first;
        serilisable = !find_cycle(root, visited, parent, number_hist, conflicts_set);
    } else {
        serilisable = true;
    }

    printf("is serial? %d", serilisable);

    /* Recoverability analysis */
    bool recoverable = true;
    bool aca = true;
    bool st = true;
    for (int i = 0; i < num_target; i++) {
        if (target[i].op == 0) {
            /* look for dirty read */
            int write_index = get_write(target[i], target, i);
            if (write_index < 0) {
                continue;
            } 
            /* check for commit to confirm dirty read */ 
            if (is_dirty(write_index, i, target)) {
                aca = false;
            }
            if (!check_commit_order(target[write_index], target[i], target)) {
                // found a dirty read with a incorrect commit order
                recoverable = false; 
                break;
            }
        } 
        else if (target[i].op == 1) {
            /* look for dirty write */
            int write_index = get_write(target[i], target, i);
            if (write_index < 0) {
                continue;
            } 
            /* check for commit to confirm dirty write */ 
            if (is_dirty(write_index, i, target)) {
                st = false;
            }
        }
    }
    

    /* write out to file for visualisation */
    FILE *file_out = fopen("out", "w");
    if (!file) {
        perror("Cant open file for reading \n");
        exit(EXIT_FAILURE);
    }

    /* first line contains whether the graph is serialisable and if its recoverable or ACA or ST */
    fprintf(file_out, "%s and %s\n", serilisable? "Serialisable":"not serialisable", recoverable? (aca? (st? "ST":"ACA") :"Recoverable" ):"Not recoverable");

    /* then write out the conflicts for the graph */
    for (int i = 0; conflicts_set[i]; i++) {
        fprintf(file_out, "%d,%d\n", conflicts_set[i]->first, conflicts_set[i]->second);
    }


    /* free allocs */
    free_spine((void **) history_arr, number_hist + 1);
    free(num_items);
    free_spine((void **) object_names, num_target);
    free_spine((void **) conflicts, max_conflicts);
    free(conflicts_set);
    free(parent);
    free(visited);
    
    
    /* GOODBYE! */
    fclose(file);
    fclose(file_out);
    exit(EXIT_SUCCESS);
}

/* finds conflicts between items */
bool is_conflict(item *first, item* second) {
    if (first->op > 1 || second->op > 1) {
        return false;
    }
    if (first->op == 0 && second->op == 0) {
        return false;
    }
    if (!same_obj(first, second)) {
        return false;
    }
    if (first->num == second->num) {
        return false;
    }
    return true;
}
/*checks if both items operate on the same object*/
bool same_obj(item *first, item* second) {
    return !strcmp(first->name, second->name);
}
/*checks if item has the same name as the string passed in*/
bool same_name(item element, char *name) {
    return !strcmp(element.name, name);
}

int nc2(int num) {
    return num * (num - 1) / 2;
}
/* checks if two conflicts are the same or not */
bool compare_conflicts(conflict first_con, conflict second_con) {
    return first_con.first == second_con.first && first_con.second == second_con.second;
}

/* sum array num items */
int sum_array(int *num_items, int len) {
    int res = 0;
    for (int i = 1; i <= len; i++) {
        res += num_items[i];
    }
    return res;
}

/* parse each line and place in history array */
int parse_line(item **history_arr, char *line, int index) {

    /* get total token count */
    int count = 1;
    for (int i = 0; line[i  ]; i++) {
        if (line[i] == ',') {
            count++;
        }
    }

    item *history = malloc(count * sizeof(item));
    char *token = strtok(line, ", ");
    count = 0;

    /* set history op type */
    while (token) {
        switch (token[0]) {
        case 'r':
            history[count].op = READ;
            break;
        case 'w':
            history[count].op = WRITE;
            break;
        case 'c':
            history[count].op = COMMIT;
            break;
        case 'a':
            history[count].op = ABORT;
            break;
        default:
            perror("incorrect input given to parser \n");
            exit(EXIT_FAILURE);
        }

        /* set history number */
        char str_num[MAX_DIGIT_SIZE];
        int i = 1;
        for (; token[i] && token[i] != '('; i++) {
            str_num[i - 1] = token[i];
        }
        history[count].num = atoi(str_num);
        
        if (history[count].op == READ || history[count].op == WRITE) {
            /* get the object name */
            for (int j = i + 1; token[j] != ')'; j++) {
                history[count].name[j - i - 1] = token[j];
            }
        } 
        count++;
        token = strtok(NULL, ", ");
    }
    /* put item in history array */
    history_arr[index] = history; 

    return count;  
}

/* update in 'set' fashion */
void update_names(char **object_name, char* name) {
    int i = 0;
    for (; object_name[i]; i++) {
        if (!strcmp(object_name[i], name)) {
            return;
        }
    }
    char *obj = malloc(sizeof(name));
    strcpy(obj, name);
    object_name[i] = obj;
}

/* get unique conflicts */
void get_set(conflict** conflicts_set, conflict** conflicts) {
    for (int i = 0; conflicts[i]; i++) {
        bool contains = false;
        int j = 0;
        for (; conflicts_set[j]; j++) {
            if (compare_conflicts(*conflicts_set[j], *conflicts[i])) {
                contains = true;
                break;
            }
        }
        if (!contains) {
            conflicts_set[j] = conflicts[i];
        }
    }
}


bool find_cycle(int root, bool visited[], int parent[], int len, conflict **conflict_set) {
    visited[root] = true;

    /* get adjacent nodes */
    int adj[len];
    int index = 0;
    for (int i = 0; conflict_set[i]; i++) {
        if (conflict_set[i]->first == root) {
            adj[index] = conflict_set[i]->second;
            index++;
        } 
    }

    for (int i = 0; i <= index; i++) {
        if (visited[adj[i]] && adj[i] != parent[root]) {
            return true;
        }
        if (!visited[adj[i]]) {
            parent[adj[i]] = root;
            return find_cycle(adj[i], visited, parent, len, conflict_set);
        }
    }

    perror("DFS FAILED \n");
    exit(EXIT_FAILURE);
}

/*
* Returns the index of a conflicting write with the object before the given index
* Returns negative for not found case
*/
int get_write(item object, item* target, int index) {
    for (int i = 0; i < index; i++) {
        if (!(strcmp(target[i].name ,object.name)) && target[i].num != object.num
                && target[i].op == 1) {
                    return i;
        }
    }
    return -1;
}

/* checks if commit order is correct for recoverability */
bool check_commit_order(item write, item read, item *target) {
    int write_num = write.num;
    int read_num = read.num;

    int write_commit_index = -1;
    int read_commit_index = -1;

    int index = 0;
    while (write_commit_index < 0 || read_commit_index < 0)
    {   
        item obj = target[index];
        if (obj.num == write_num && (obj.op == 2 || obj.op == 3)) {
            write_commit_index = index;
        }
        if (obj.num == read_num && (obj.op == 2 || obj.op == 3)) {
            read_commit_index = index;
        }
        index++;
    }
    
    return write_commit_index < read_commit_index;
    
}

/* checks if there is a dirty reads between items at write/read indices */
bool is_dirty(int write_index, int read_index, item *target) {
    item write = target[write_index];
    for (int i = write_index + 1; i < read_index; i++) {
        if (target[i].op > 1 && target[i].num == write.num) {
            return false;
        }
    }
    return true;
}

/* free a spine like array starting at a particular index */
void free_spine(void **arr, int len) {
    for (int i = 0; i < len; i++) {
        free(arr[i]);
    }
    free(arr);
}






