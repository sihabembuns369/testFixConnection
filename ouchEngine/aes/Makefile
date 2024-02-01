#include ../../Makefile.inc

CFLAGS = -fPIE -c -Wall -O3 --std=c++11 -I. 

CASM = aes_x86_v1 aes_x86_v2 \

CPGM = aescrypt aeskey aes_modes aes_ni aestab aesxam tablegen \

all: $(CPGM) $(CASM)
#	$(CC) $(CFLAGS) $@.c

$(CASM) : $@
	nasm -felf32 -g -Fdwarf $@.asm -o $@.o

$(CPGM) : $@
	$(CC) $(CFLAGS) $@.c

asm : $(CASM)

clean :
	rm *.o
