ALFLAGS=-i altlib ../../altlib/ -t
CFLAGS=-I ../../stdlib/include
LDFLAGS=-z noexecstack -e test

SRCS_AL := $(wildcard *.al)
EXES_x86_64 := $(patsubst %.al, $(BUILD_DIR)/test/unit/x86_64/%, $(SRCS_AL))
EXES_C := $(patsubst %.al, $(BUILD_DIR)/test/unit/c/%, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)
OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(EXES_x86_64) $(EXES_C) docs

$(BUILD_DIR)/test/unit/x86_64/%: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@.o
	ld $(LDFLAGS) -o $@ $@.o $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/unit/c/%: %.al make_dir
	$(BUILD_DIR)/calias -a -b c $(ALFLAGS) $< -o $@.c
	gcc -fno-stack-protector -c $@.c -o $@.o -no-pie
	ld $(LDFLAGS) -o $@ $@.o $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/unit/generate_docs: generate_docs.c
	gcc $(CFLAGS) $< $(OBJS_ARCH) $(OBJS_STDLIB) -o $@

docs: $(BUILD_DIR)/test/unit/generate_docs
	cat header.html > ../../docs/langref.html
	$< $(SRCS_AL) >> ../../docs/langref.html

make_dir:
	mkdir -p $(BUILD_DIR)/test/unit/x86_64
	mkdir -p $(BUILD_DIR)/test/unit/c
