ALFLAGS=-i altlib ../../altlib/ -t
CFLAGS=-I ../../stdlib/include
LDFLAGS=-z noexecstack -e test

SRCS_AL := $(wildcard *.al)
EXES_AL := $(patsubst %.al, $(BUILD_DIR)/test/unit/%, $(SRCS_AL))

OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(EXES_AL) docs

$(BUILD_DIR)/test/unit/%.o: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@

$(BUILD_DIR)/test/unit/%: $(BUILD_DIR)/test/unit/%.o make_dir
	ld $(LDFLAGS) -o $@ $< $(OBJS_STDLIB) $(OBJS_ALTLIB)
	$@ || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/unit/generate_docs: generate_docs.c
	gcc $(CFLAGS) $< $(OBJS_STDLIB) -o $@

docs: $(BUILD_DIR)/test/unit/generate_docs
	cat header.html > ../../docs/langref.html
	$< $(SRCS_AL) >> ../../docs/langref.html

make_dir:
	mkdir -p $(BUILD_DIR)/test/unit

clean:
	rm -r $(BUILD_DIR)/test
