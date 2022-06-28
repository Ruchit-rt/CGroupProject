CC      = gcc
CFLAGS  = -Wall -g -D_POSIX_SOURCE -D_DEFAULT_SOURCE -std=c99 -Werror -pedantic

.SUFFIXES: .c .o
.PHONY: all clean

all: trans_Analyser 

trans_Analyser.o: trans_Analyser.h

clean:
	rm -f $(wildcard *.o */*.o)
	rm -f trans_Analyser
	rm -f out
