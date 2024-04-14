SRCS_ASM := $(wildcard asm/*.asm)
OBJS_ASM := $(patsubst %.asm, build/%.o, $(SRCS_ASM))

SRCS_C := $(wildcard src/*.c)
OBJS_C := $(patsubst %.c, build/%.o, $(SRCS_C))

SRCS_STDLIB_C := $(wildcard stdlib/src/*.c)
OBJS_STDLIB_C := $(patsubst %.c, build/%.o, $(SRCS_STDLIB_C))

ASFLAGS=-f elf64
CFLAGS=
LDFLAGS=

.PHONY: all test

all: make_dir $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_C) link

make_dir:
	mkdir -p build/asm
	mkdir -p build/src
	mkdir -p build/stdlib/src

build/%.o: %.asm
	nasm $(ASFLAGS) $< -o $@

build/%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

link:
	gcc $(LDFLAGS) -o build/calias $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_C) -z noexecstack

test:
	./build/calias test/main.al -m -c
	nasm -f elf64 test/main.asm -o test/main.o
	gcc test/malloc.c -c -o test/malloc.o
	ld test/main.o test/malloc.o -o test/main -z noexecstack

run:
	./test/main

clean:
	rm -r build