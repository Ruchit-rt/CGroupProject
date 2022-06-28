CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o
.PHONY: all clean

all: assemble emulate

assemble: assemble.o assemble_tools/tokenizer.o assemble_tools/symbol_table.o assemble_tools/parser.o assemble_tools/word_arr.o assemble_tools/assembler.o
emulate: emulate.o emulate_tools/decode.o emulate_tools/execute.o

emulate.o: emulate.c defns.h emulate_tools/decode.h emulate_tools/state_of_sys.h emulate_tools/execute.h
emulate_tools/decode.o: emulate_tools/decode.h emulate_tools/decode.c instruction.h
emulate_tools/execute.o: emulate_tools/execute.h emulate_tools/state_of_sys.h defns.h instruction.h

assemble.o: defns.h assemble_tools/assemble_st.h assemble_tools/tokenizer.h assemble_tools/symbol_table.h assemble_tools/parser.h assemble_tools/word_arr.h
assemble_tools/tokenizer.o: assemble_tools/tokenizer.h assemble_tools/assemble_st.h defns.h 
assemble_tools/symbol_table.o: assemble_tools/tokenizer.h
assemble_tools/parser.o: assemble_tools/parser.h assemble_tools/assemble_st.h assemble_tools/word_arr.h
assemble_tools/word_arr.o: assemble_tools/word_arr.h defns.h
assemble_tools/assembler.o: assemble_tools/assembler.h assemble_tools/assemble_st.h

clean:
	rm -f $(wildcard *.o */*.o)
	rm -f assemble
	rm -f emulate
