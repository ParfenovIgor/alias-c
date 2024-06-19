SRCS_ASM := $(wildcard asm/*.asm)
OBJS_ASM := $(patsubst %.asm, build/%.o, $(SRCS_ASM))

SRCS_C := $(wildcard src/*.c)
OBJS_C := $(patsubst %.c, build/%.o, $(SRCS_C))

SRCS_STDLIB_ASM := $(wildcard stdlib/asm/*.asm)
OBJS_STDLIB_ASM := $(patsubst %.asm, build/%.o, $(SRCS_STDLIB_ASM))

SRCS_STDLIB_C := $(wildcard stdlib/src/*.c)
OBJS_STDLIB_C := $(patsubst %.c, build/%.o, $(SRCS_STDLIB_C))

SRCS_TEST_AL := $(wildcard test/src/*.al)
OBJS_TEST_AL := $(patsubst %.al, build/%.o, $(SRCS_TEST_AL))

ASFLAGS=-f elf64
CFLAGS=-g
LDFLAGS=

.PHONY: all compiler test

all: calias

calias: make_dir $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) link

make_dir:
	mkdir -p build/asm
	mkdir -p build/src
	mkdir -p build/stdlib/asm
	mkdir -p build/stdlib/src
	mkdir -p build/test/src

build/%.o: %.asm
	nasm $(ASFLAGS) $< -o $@

build/%.o: %.c
	gcc $(CFLAGS) -c $< -o $@ -fno-stack-protector

build/%.o: %.al
	build/calias -a $< -o $@

link:
	gcc $(LDFLAGS) -o build/calias $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) -z noexecstack

run:
	./test/main

test: make_dir $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) $(OBJS_TEST_AL) link_test

link_test:
	ld $(LDFLAGS) -o build/testal $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) $(OBJS_TEST_AL) -z noexecstack

clean:
	rm -r build
