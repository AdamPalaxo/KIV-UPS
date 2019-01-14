CC=gcc
CFLAGS=-ansi -Wall -Wextra -pedantic -pthread

all:	clean client server

server: server.c
	${CC} ${CFLAGS} -o server server.c player.c game.c board.c utils.c

clean: 
	rm -f server


