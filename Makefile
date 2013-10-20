CFLAGS = -Wall -g
SOURCES = cpu/cpu.c main.c

all: emu

emu:
	gcc $(CFLAGS) $(SOURCES) -o lc3-emu
