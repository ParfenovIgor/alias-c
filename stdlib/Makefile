ASFLAGS=-f elf64
CFLAGS=-g -fno-stack-protector -I include

SRCS_ASM := $(wildcard asm/*.asm)
OBJS_ASM := $(patsubst asm/%.asm, $(BUILD_DIR)/stdlib/%.o, $(SRCS_ASM))

SRCS_C := $(wildcard src/*.c)
OBJS_C := $(patsubst src/%.c, $(BUILD_DIR)/stdlib/%.o, $(SRCS_C))

all: $(OBJS_ASM) $(OBJS_C)

$(BUILD_DIR)/stdlib/%.o: asm/%.asm make_dir
	nasm $(ASFLAGS) $< -o $@

$(BUILD_DIR)/stdlib/%.o: src/%.c make_dir
	gcc $(CFLAGS) -c $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/stdlib

clean:
	rm -r $(BUILD_DIR)/stdlib
