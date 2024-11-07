CC = gcc
FLAGS= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
.PHONY: all clean zip config_doxygen create_doxygen

all: mygrep

mygrep: mygrep.o
	$(CC)  $(FLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(FLAGS) -c -o $@ $<

mygrep.o: mygrep.c mygrep.h

clean:
	rm -rf *.o mygrep ex1a.tar.gz

zip:
	tar -cvzf ex1a.tar.gz Makefile *.c *h

config_doxygen:
	doxygen -g mygrep_doxygen  
create_doxygen:
	doxygen mygrep_doxygen