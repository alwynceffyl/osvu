# Makefile für HTTP-Client und Server
# @author Phillip Sassmann
# @brief Build-Skript für den HTTP-Client und -Server.
# @date 2024-12-22

CC = gcc
FLAGS= -std=gnu99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g
LDFLAGS = -pthread

.PHONY: all clean zipClient #zipServer

all: client #server

client: client.o 
	$(CC)  $(FLAGS) -o $@ $^ $(LDFLAGS)

#server: server.o
#	$(CC)  $(FLAGS) -o $@ $^ $(LDFLAGS)
	
%.o: %.c %.h
	$(CC) $(FLAGS) -c -o $@ $<

client.o: client.c client.h
#server.o: server.c server.h


clean:
	rm -rf *.o client server ex3a.tar.gz ex3b.tar.gz

zipClient:
	tar -cvzf ex3a.tar.gz Makefile client.c client.h

#zipServer:
#	tar -cvzf ex3b.tar.gz Makefile server.c server.h
