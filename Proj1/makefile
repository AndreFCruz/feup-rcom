# Makefile for RCOM - Project 1

COMPILER_TYPE = gnu
CC = gcc

PROG = app
SRCS = main.c utils.c ApplicationLayer.c Packets.c LinkLayer.c Alarm.c

CFLAGS= -Wall -g

$(PROG): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(PROG)

clean:
	rm -f $(PROG)