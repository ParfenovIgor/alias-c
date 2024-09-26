SRCS_ASM := $(wildcard asm/*.asm)
OBJS_ASM := $(patsubst %.asm, build/%.o, $(SRCS_ASM))

SRCS_C := $(wildcard src/*.c)
OBJS_C := $(patsubst %.c, build/%.o, $(SRCS_C))

SRCS_STDLIB_ASM := $(wildcard stdlib/asm/*.asm)
OBJS_STDLIB_ASM := $(patsubst %.asm, build/%.o, $(SRCS_STDLIB_ASM))

SRCS_STDLIB_C := $(wildcard stdlib/src/*.c)
OBJS_STDLIB_C := $(patsubst %.c, build/%.o, $(SRCS_STDLIB_C))

SRCS_ALTLIB_AL := $(wildcard altlib/src/*.al)
OBJS_ALTLIB_AL := $(patsubst %.al, build/%.o, $(SRCS_ALTLIB_AL))

SRCS_TEST_AL := $(wildcard test/src/*.al)
OBJS_TEST_AL := $(patsubst %.al, build/%.o, $(SRCS_TEST_AL))

ASFLAGS=-f elf64
CFLAGS=-g -fno-stack-protector
LDFLAGS=-z noexecstack

compiler: build/calias

build/calias: make_dir $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C)
	gcc $(LDFLAGS) -o build/calias $(OBJS_ASM) $(OBJS_C) $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C)

build/%.o: %.asm make_dir
	nasm $(ASFLAGS) $< -o $@

build/%.o: %.c make_dir
	gcc $(CFLAGS) -c $< -o $@

build/%.o: %.al make_dir
	build/calias -a $< -o $@

test: make_dir $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) $(OBJS_ALTLIB_AL) $(OBJS_TEST_AL)
	ld $(LDFLAGS) -o build/testal $(OBJS_STDLIB_ASM) $(OBJS_STDLIB_C) $(OBJS_ALTLIB_AL) $(OBJS_TEST_AL)
	./build/testal > build/test/output
	diff build/test/output test/output || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p build/asm
	mkdir -p build/src
	mkdir -p build/stdlib/asm
	mkdir -p build/stdlib/src
	mkdir -p build/altlib/src
	mkdir -p build/test/src

clean:
	rm -r build
