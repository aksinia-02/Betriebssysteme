#Makefile for myexpand
#@author Vorobeva Aksinia 12044614
#@date 13.11.2023

CC = gcc
DEFS =  -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)
LDFLAGS =

OBJECTS = myexpand.o

.PHONY: all clean

all: myexpand

myexpand: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
myexpand.o: myexpand.c

clean:
	rm -rf *o myexpand
