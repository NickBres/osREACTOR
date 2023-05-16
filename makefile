CC = gcc

all: reactor server client

server: reactor_server.c reactor_server.h
	$(CC) -o server reactor_server.c -L. -lst_reactor -lpthread

client: client.c
	$(CC) -o client client.c

reactor: st_reactor.c st_reactor.h
	$(CC) -c -fPIC st_reactor.c -o st_reactor.o
	$(CC) -shared -o libst_reactor.so st_reactor.o

clean:
	rm -rf *.o server *.so client
