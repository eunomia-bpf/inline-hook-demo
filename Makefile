CC=gcc
ARM32CC=arm-linux-gnueabi-gcc
ARM64CC=aarch64-linux-gnu-gcc
CFLAGS=-g -fno-pie -no-pie
ARMCFLAGS=-g -fpie
SRC = main.c hook.c funcaddr.c

.PHONY: all clean arm32 arm64 x86

all: maps.off.txt 

maps: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o maps

maps.off.txt: maps
	nm maps | grep ' T ' > maps.off.txt

x86: maps.off.txt

arm32: maps-arm32.off.txt
	cp maps-arm32.off.txt maps.off.txt

maps-arm32: $(SRC)
	$(ARM32CC) $(ARMCFLAGS) $(SRC) -static -o maps-arm32

maps-arm32.off.txt: maps-arm32
	arm-linux-gnueabi-nm maps-arm32 | grep ' T ' > maps-arm32.off.txt

arm64: maps-arm64.off.txt
	cp maps-arm64.off.txt maps.off.txt

maps-arm64: $(SRC)
	$(ARM64CC) $(ARMCFLAGS) $(SRC) -static -o maps-arm64

maps-arm64.off.txt: maps-arm64
	aarch64-linux-gnu-nm maps-arm64 | grep ' T ' > maps-arm64.off.txt

clean:
	rm -f maps maps-arm32 maps-arm64 maps.off.txt maps-arm32.off.txt maps-arm64.off.txt
