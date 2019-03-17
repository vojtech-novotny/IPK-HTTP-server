# IPK - http server
# Makefile

# @author -	Vojtěch Novotný
# @login -	xnovot1f
# @date -	16.03.2019

CC=gcc
CFLAGS= -std=c99 -pedantic -Wall -Wextra -g

build: server.c
	$(CC) $(CFLAGS) server.c -o server
	
server: server.c
	$(CC) $(CFLAGS) server.c -o server

run: server
	./server ${port}
