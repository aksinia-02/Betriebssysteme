#Makefile for arc set
#@author Vorobeva Aksinia 12044614
#@date 11.12.2023

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS =

.PHONY: all clean

all: supervisor generator

supervisor: supervisor.o
	$(CC) $(LDFLAGS) -o supervisor supervisor.o -lpthread -lrt

generator: generator.o
	$(CC) $(LDFLAGS) -o generator generator.o -lpthread -lrt

supervisor.o: supervisor.c common.h
	$(CC) $(CFLAGS) -c -o supervisor.o supervisor.c

generator.o: generator.c common.h
	$(CC) $(CFLAGS) -c -o generator.o generator.c

clean:
	rm -rf *.o supervisor generator
