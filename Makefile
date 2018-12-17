.PHONY: all

OPT=-g
CFLAGS=-Wall -Werror $(OPT)

all: hex2bin hexinfo

hexinfo: hexinfo.o readhex.o
	$(CC) hexinfo.o readhex.o -o hexinfo

hex2bin: hex2bin.o readhex.o
	$(CC) hex2bin.o readhex.o -o hex2bin

clean:
	rm hex2bin hex2bin.o readhex.o hexinfo hexinfo.o

hex2bin.o: hex2bin.c readhex.h
hexinfo.o: hexinfo.c readhex.h
