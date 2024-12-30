ALFLAGS=-i altlib ./
CFLAGS=-I ../stdlib/include

SRCS_AL := $(wildcard *.al)
OBJS_AL := $(patsubst %.al, $(BUILD_DIR)/altlib/%.o, $(SRCS_AL))

OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(OBJS_AL) docs

$(BUILD_DIR)/altlib/%.o: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@

$(BUILD_DIR)/altlib/generate_docs: generate_docs.c
	gcc $(CFLAGS) $< $(OBJS_STDLIB) -o $@

docs: $(BUILD_DIR)/altlib/generate_docs
	cat header.html > ../docs/altlibref.html
	$< $(SRCS_AL) >> ../docs/altlibref.html

make_dir:
	mkdir -p $(BUILD_DIR)/altlib

clean:
	rm -r $(BUILD_DIR)/altlib
