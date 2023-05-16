CC = gcc

all: reactor

server: reactor_server.c reactor_server.h
	$(CC) -o server -L. -lst_reactor

reactor: st_reactor.c st_reactor.h
	$(CC) -c -fPIC st_reactor.c -o st_reactor.o
	$(CC) -shared -o st_reactor.so


clean:
	rm -rf *.o server *.so