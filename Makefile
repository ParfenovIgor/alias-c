SRCS_ASM := $(wildcard asm/*.s)
OBJS_ASM := $(patsubst %.s, build/%.o, $(SRCS_ASM))

SRCS_C := $(wildcard source/*.c)
OBJS_C := $(patsubst %.c, build/%.o, $(SRCS_C))

ASFLAGS=-f elf64
CFLAGS=
LDFLAGS=

.PHONY: all test

all: make_dir $(OBJS_ASM) $(OBJS_C) link

make_dir:
	mkdir -p build/asm
	mkdir -p build/source

build/%.o: %.s
	nasm $(ASFLAGS) $< -o $@

build/%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

link:
	gcc $(LDFLAGS) -o build/calias $(OBJS_ASM) $(OBJS_C)

test:
	./build/calias test/main.al -m -a
	gcc test/malloc.c -c -o test/malloc.o
	ld test/main.o test/malloc.o -o test/main

run:
	./test/main

clean:
	rm -r build