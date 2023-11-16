CC=gcc
CFLAGS=-I.

all : client server

libmultmodulo.a: MultModulo.c
	$(CC) -c MultModulo.c $(CFLAGS) 
	ar rcs libmultmodulo.a MultModulo.o

client: client.c libmultmodulo.a servers.txt
	$(CC) -pthread client.c -o client -L. -lmultmodulo $(CFLAGS)

server: server.c libmultmodulo.a
	$(CC) -pthread server.c -o server -L. -lmultmodulo $(CFLAGS)

clean :
	rm -f client server libmultmodulo.a *.o
