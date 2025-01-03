ASFLAGS=-f elf64

SRCS_ASM := $(wildcard x86/*.asm)
OBJS_ASM := $(patsubst x86/%.asm, $(BUILD_DIR)/arch/%.o, $(SRCS_ASM))

all: $(OBJS_ASM) $(OBJS_C)

$(BUILD_DIR)/arch/%.o: x86/%.asm make_dir
	nasm $(ASFLAGS) $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/arch
