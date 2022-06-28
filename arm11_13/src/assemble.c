#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "defns.h"
#include "assemble_tools/assemble_st.h"
#include "assemble_tools/tokenizer.h"
#include "assemble_tools/symbol_table.h"
#include "assemble_tools/parser.h"
#include "assemble_tools/word_arr.h"
#include "assemble_tools/assembler.h"

typedef word_t (* assemble_func)(ass_t *);

/* declarations */
char **create_spine_array(int cols, int rows);
int number_of_lines(char *filename);
char **read_file(char *filename, int lines);
void free_spine_array(char **file_buffer, int rows);
mne_t get_mne(char *str);
void set_inst_type(ass_t *ass);
static assemble_func get_assm_func(inst_type_t opcode);
static void write_out(FILE *file_out, word_t out);

int main(int argc, char **argv) {
  assert(argc == 3);
  char *file_in = argv[1];
  char *file_out_name = argv[2];
  
  int num_lines = number_of_lines(file_in);
  printf("num lines : %d ---->\n\n", num_lines);

  char **file_buffer = read_file(file_in, num_lines);

  /* create symbol table thru first pass */
  new_buffer *new_buff = generate_symbol_table(file_buffer, num_lines);
  char **new_file_buffer = new_buff->new_file_buffer;
  int new_num_lines = new_buff->new_num_lines;
  label_map *symbol_table = new_buff->map;

  /* testing new file buffer */
  for (int i = 0; i < new_num_lines; i++) {
    printf("Line %d: %s \n", i, new_file_buffer[i]);
  }
  /* testing symbol table */
  for (int i = 0; i < symbol_table->size; i++) {
    label_t *label_row = symbol_table->labels[i];
    printf("Label: %s, Addr: %u \n", label_row->label, label_row->address);
  }

  /* creating word_arr_t for storing EOF words */
  word_arr_t *words = make_word_arr();

  /* opening out file */
  FILE *file_out = fopen(file_out_name, "w");
  if (!file_out) {
    perror("could not open out file \n");
    exit(EXIT_FAILURE);
  }

  printf("New number of lines are %d \n", new_num_lines);

  /* seconnd pass */
  for (int i = 0; i < new_num_lines; i++) {
      printf("i = %d \n", i);
      // TODO - dont calloc every time, but default everything
      ass_t *ass = calloc(1, sizeof(ass_t));
      tokens_t *tokens = tokenize(new_file_buffer[i]);
      ass->mn_type = get_mne(tokens->array_tokens[0]);
      set_inst_type(ass);
      switch (ass->inst_type) {
      case DP:
        dp_parser(ass, tokens);
        break;
      case MUL:
        mul_parser(ass, tokens);
        break;
      case SDT:
        if (ass->mn_type == STRm && ass->inst_type == SDT) {
          printf("CORRECT \n");
        }
        sdt_parser(ass, tokens, words, i, new_num_lines);
        break;
      case BRA:
        branch_parser(ass, tokens, symbol_table, i << 2);
        break; 
      case SPL:
        special_parser(ass, tokens);
        break; 
      default:
        perror("Incorrect instruction type \n");
        break;
      }

      word_t out;
      out = (get_assm_func(ass->inst_type))(ass);
      printf("out: %x \n", out);

      /* writing out to binary file */
      write_out(file_out, out);

      /* testing parser */
      printf("\nTesting structure\n");
      printf("OP = %d \n", ass->op);
      printf("COND = 0x%x \n", ass->cond);
      printf("RD = 0x%x \n", ass->rd);
      printf("RN = 0x%x \n", ass->rn);
      printf("RM = 0x%x \n", ass->rm);
      printf("RS = 0x%x \n", ass->rs);
      printf("OP2 = 0x%x \n", ass->op2);
      printf("SL = %x \n", ass->sl);
      printf("I = %x \n", ass->i);
      printf("P = %x \n", ass->p);
      printf("UA = %x \n", ass->ua);
      printf("offset = %x \n", ass->offset);
      printf("branch_offset = %x\n", ass->branch_offset);
      free_tokens(tokens);
      free(ass);
  }

  /* writing extra words and closing out file */
  for (int i = 0; i < words->size; i++) {
    write_out(file_out, words->arr[i]);
  }
  fclose(file_out);

  /* freeing alloc's */
  free_word_arr(words);
  free_spine_array(file_buffer, num_lines);
  free_buffer(new_buff);
  
  return EXIT_SUCCESS;
}

static void write_out(FILE *file_out, word_t out) {
  for (int i = 0; i < 4; i++) {
    byte_t output = (out >> (i * 8)) & 0xFF;
    fwrite(&output, sizeof(byte_t), 1, file_out);
  }
}

mne_t get_mne(char *str) {
  GET_ENUM("add", ADDm);
  GET_ENUM("sub", SUBm);
  GET_ENUM("rsb", RSBm);
  GET_ENUM("and", ANDm);
  GET_ENUM("eor", EORm);
  GET_ENUM("orr", ORRm);
  GET_ENUM("mov", MOVm);
  GET_ENUM("tst", TSTm);
  GET_ENUM("teq", TEQm);
  GET_ENUM("cmp", CMPm);
  GET_ENUM("mul", MULm);
  GET_ENUM("mla", MLAm);
  GET_ENUM("ldr", LDRm);
  GET_ENUM("str", STRm);
  GET_ENUM("beq", BEQm);
  GET_ENUM("bne", BNEm);
  GET_ENUM("bge", BGEm);
  GET_ENUM("blt", BLTm);
  GET_ENUM("bgt", BGTm);
  GET_ENUM("ble", BLEm);
  GET_ENUM("b", Bm);
  GET_ENUM("bal", Bm);
  GET_ENUM("lsl", LSLm);
  GET_ENUM("andeq", ANDEQm);
  fprintf(stderr, "COULD NOT MATCH MNE \n");
  exit(EXIT_FAILURE);
}

void set_inst_type(ass_t *ass) {
  switch (ass->mn_type) {
  case ANDm:
  case EORm:
  case SUBm:
  case RSBm:
  case ADDm:
  case TSTm:
  case TEQm:
  case CMPm:
  case ORRm:
  case MOVm:
    ass->inst_type = DP;
    break;
  case LDRm:
  case STRm:
    ass->inst_type = SDT;
    break;
  case MULm:
  case MLAm:
    ass->inst_type = MUL;
    break;
  case BEQm:
  case BNEm:
  case BGEm:
  case BLTm:
  case BGTm:
  case BLEm:
  case Bm:
    ass->inst_type = BRA;
    break;
  case ANDEQm:
  case LSLm:
    ass->inst_type = SPL; 
    break;
  default:
    fprintf(stderr, "ENUM type not matched \n");
    exit(EXIT_FAILURE);
    break;
  }
}

int number_of_lines(char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Cant read from provided filename");
  }
  char line[max_char_inline];
  int count = 0;

  while ((fgets(line, max_char_inline, file) != NULL)) {
    if (strlen(line) > 1) {
      count++;
    }
  }
  fclose(file);
  return count;
}

char **create_spine_array(int cols, int rows) {
    char ** file_buffer = malloc(rows * sizeof(char *));
    if (!file_buffer) {
      fprintf(stderr, "Could not allocate file buffer \n");
      exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
      file_buffer[i] = malloc(cols * sizeof(char));
      if (!file_buffer[i]) {
        fprintf(stderr, "Could not allocate file buffer \n");
        exit(EXIT_FAILURE);
      }
    }
  return file_buffer;
}

static assemble_func get_assm_func(inst_type_t opcode) {
  switch(opcode) {
    case DP:
      return dp_assembler;
    case MUL:
      return mul_assembler;
    case SDT:
      return sdt_assembler;
    case BRA:
      return branch_assembler;
    default:
      perror("Cannot match inst type \n");
      exit(EXIT_FAILURE);
  };
}

char **read_file(char *filename, int lines) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Cant read from provided filename");
  }

  char **file_buffer = create_spine_array(max_char_inline, lines);

  char line[max_char_inline];
  int counter = 0;
  while (fgets(line, max_char_inline, file) != NULL) {
      if(strlen(line) > 1) {
        line[strlen(line) - 1] = '\0';
        strcpy(file_buffer[counter], line);
        counter++;
      }      
  }
  fclose(file);
  return file_buffer;
}