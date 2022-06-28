#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "defns.h"
#include "emulate_tools/decode.h"
#include "emulate_tools/state_of_sys.h"
#include "emulate_tools/execute.h"

/* A null (non-existant) instruction. */
static const inst_t NULL_INSTRUCTION = {
  .type = NUL,
  .cond = 0,
  .shift_amount = 0,
  .i = false,
  .p = false,
  .ua = false,
  .sl = false,
  .rm = 0,
  .rd = 0,
  .rs = 0,
  .rn = 0,
};

/* A defautl state for the mem/reg */
static const state_sys_t DEFAULT_SYSTEM_STATE = {
  .registers = {0},
  .memory = {0},
  .fetched_inst = 0,
  .has_fetched = false,
};


/* declarations */
void readFile(char *filename, byte_t *memHeap);
word_t process(byte_t *mem);
word_t fetch(byte_t *mem);
void printState(state_sys_t *state);
void terminate(state_sys_t *state);


int main(int argc, char **argv) {
  assert(argc == 2);
  char *fileName = argv[1];

  /* creating and intialising system state */
  state_sys_t *state = malloc(sizeof(state_sys_t));

  /* giving state default values */
  *state = DEFAULT_SYSTEM_STATE;
  state->decoded_inst = malloc(sizeof(inst_t));
  *(state->decoded_inst) = NULL_INSTRUCTION;

  /* Failure in allocating memory */
  if (!state) {
    perror("Cannot allocate memory to store system_state.\n");
    return EXIT_FAILURE;
  }

  /* reading bytes from file into mem buffer */
  readFile(fileName, state->memory);

  /* Main Pipeline Loop */
  int finish = 0;
  while (finish >= 0 && state->decoded_inst->type != HAL) {

    if (state->decoded_inst->type != NUL) {
      finish = execute(state);
      if (finish < 0) {
        terminate(state);
        break;
      }
    }

    *(state->decoded_inst) = NULL_INSTRUCTION;
    if (state->has_fetched) {  
      decode(state);
      state->has_fetched = false;
    }

    /* fetch cycle */
    word_t fetched = process(state->memory + state->registers[PC]);
    
    /* increment PC */
    state->registers[PC] += 4;

    /* NEED TO DECODE ON NEXT ITERATION */
    state->fetched_inst = fetched;
    state->has_fetched = true;
  }

  /* GOODBYE! */
  terminate(state);

  return EXIT_SUCCESS;
}

void readFile(char *filename, byte_t *memHeap) {
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Cant read from provided filename");
  }
  size_t size = fread(memHeap, NUM_OF_ADDRS, 1, file);

  if (ferror(file)) {
     printf("File size: %lu", size);
     fprintf(stderr, "Error in reading from object code file. \n");
     exit(EXIT_FAILURE);
   }

  fclose(file);
}

void printState(state_sys_t *state) {
  printf("Registers:\n");
  for (int i = 0; i < NUM_OF_REGS; i++) {
    word_t reg = state->registers[i];

    if (i < 10) {
      printf("$%-3d: %10d (0x%08x)\n", i, reg, reg);
    }
    else if (i < 13) {
      printf("$%-3d: %10d (0x%08x)\n", i, reg, reg);
    }
    else if (i == 15) {
      printf("%-4s: %10d (0x%08x)\n","PC",reg, reg);
    }
    else if (i == 16) {
      printf("%-4s: %10d (0x%08x)\n","CPSR",reg, reg);
    }
  }
  printf("Non-zero memory:\n");
  byte_t *memPtr = state->memory;
  for (int i = 0; i < NUM_OF_ADDRS; i = i+4) {
    /* fetching memory words stored in little endian order */
    word_t value = fetch(memPtr + i);
    if (value) {
      printf("0x%08x: 0x%08x\n", i, value);
    }
  }
}

/* process bytes in little endian order and return word */
word_t process(byte_t *mem) {
  word_t value; 
  for (int i = 3; i >= 0; i--) {
    value = (value << 8)| *(mem + i);
  }
  return value;
}

/* process bytes in big endian order and return word */
word_t fetch(byte_t *mem) {
  word_t value; 
  for (int i = 0; i < 4; i++) {
    value = (value << 8)| *(mem + i);
  }
  return value;
}

void terminate(state_sys_t *state) {
  /* print state status */
  printState(state);

  /* freeing heap */
  free(state->decoded_inst);
  free(state);
}