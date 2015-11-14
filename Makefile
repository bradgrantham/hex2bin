.PHONY: all

OPT=-g
CFLAGS=-Wall $(OPT)

all: hex2bin

hex2bin: hex2bin.o readhex.o
	$(CC) hex2bin.o readhex.o -o hex2bin

clean:
	rm hex2bin hex2bin.o readhex.o

hex2bin.o: hex2bin.c readhex.h
