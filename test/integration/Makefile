ALFLAGS=-i altlib ../../altlib/
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_x86_64_LEGACY := $(patsubst %.al, $(BUILD_DIR)/test/integration/x86_64_legacy/%, $(SRCS_AL))
EXES_x86_64 := $(patsubst %.al, $(BUILD_DIR)/test/integration/x86_64/%, $(SRCS_AL))
EXES_C := $(patsubst %.al, $(BUILD_DIR)/test/integration/c/%, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(EXES_x86_64) $(EXES_x86_64_LEGACY) $(EXES_C)

$(BUILD_DIR)/test/integration/x86_64_legacy/%: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@.o
	ld $(LDFLAGS) -o $@ $@.o $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ > $(patsubst %, %.out, $@)
	diff $(patsubst %, %.out, $@) $(patsubst %.al, %.out, $<) || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/integration/x86_64/%: %.al make_dir
	$(BUILD_DIR)/calias -a -b x86_64_asm $(ALFLAGS) $< -o $@.asm
	nasm -f elf64 $@.asm -o $@.o
	ld $(LDFLAGS) -o $@ $@.o $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ > $(patsubst %, %.out, $@)
	diff $(patsubst %, %.out, $@) $(patsubst %.al, %.out, $<) || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/integration/c/%: %.al make_dir
	$(BUILD_DIR)/calias -a -b c $(ALFLAGS) $< -o $@.c
	gcc -fno-stack-protector -g -c $@.c -o $@.o -no-pie
	ld $(LDFLAGS) -o $@ $@.o $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ > $(patsubst %, %.out, $@)
	diff $(patsubst %, %.out, $@) $(patsubst %.al, %.out, $<) || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p $(BUILD_DIR)/test/integration/x86_64_legacy
	mkdir -p $(BUILD_DIR)/test/integration/x86_64
	mkdir -p $(BUILD_DIR)/test/integration/c
