ALFLAGS=-i altlib ./
CFLAGS=-I ../stdlib/include

SRCS_AL := $(wildcard *.al)
OBJS_AL := $(patsubst %.al, $(BUILD_DIR)/altlib/%.o, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(OBJS_AL) docs
	cp $(SRCS_AL) $(BUILD_DIR)/altlib_ext/include
	cp $(OBJS_AL) $(BUILD_DIR)/altlib_ext/lib
	cp $(OBJS_ARCH) $(BUILD_DIR)/altlib_ext/lib

$(BUILD_DIR)/altlib/%.o: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@

$(BUILD_DIR)/altlib/generate_docs: generate_docs.c
	gcc $(CFLAGS) $< $(OBJS_ARCH) $(OBJS_STDLIB) -o $@

docs: $(BUILD_DIR)/altlib/generate_docs
	cat header.html > ../docs/altlibref.html
	$< $(SRCS_AL) >> ../docs/altlibref.html

make_dir:
	mkdir -p $(BUILD_DIR)/altlib
	mkdir -p $(BUILD_DIR)/altlib_ext/include
	mkdir -p $(BUILD_DIR)/altlib_ext/lib
