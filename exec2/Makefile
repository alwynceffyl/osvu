CC = gcc
FLAGS= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
.PHONY: all clean zip configd created

all: supervisor generator

generator: generator.o 
	$(CC)  $(FLAGS) -o $@ $^

supervisor: supervisor.o
	$(CC)  $(FLAGS) -o $@ $^
	
%.o: %.c %.h
	$(CC) $(FLAGS) -c -o $@ $<

supervisor.o: supervisor.c common.h
generator.o: generator.c common.h


clean:
	rm -rf *.o generator supervisor ex1b.tar.gz html/ latex/ threecolor_doxygen

zip:
	tar -cvzf ex1b.tar.gz Makefile *.c *h

configd:
	doxygen -g threecolor_doxygen  
created:
	doxygen threecolor_doxygen