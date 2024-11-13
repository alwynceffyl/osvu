CC = gcc
FLAGS= -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
.PHONY: all clean zip configd created

all: forksort

forksort: forksort.o 
	$(CC)  $(FLAGS) -o $@ $^
	
%.o: %.c %.h
	$(CC) $(FLAGS) -c -o $@ $<

forksort.o: forksort.c forksort.h

clean:
	rm -rf *.o forksort ex2.tar.gz html/ latex/ forksortd

zip:
	tar -cvzf ex2.tar.gz Makefile *.c *h

configd:
	doxygen -g forksortd  
created:
	doxygen forksortd