all: build

build:
	gcc -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -o main main.c SocketManager.c ReceiveThread.c SendThread.c KeyboardThread.c PrintThread.c list/list.o -lpthread

list.o: list.h

run: build
	./main

clean: 
	rm -f main