ALFLAGS=-i altlib ./

SRCS_AL := $(wildcard *.al)
OBJS_AL := $(patsubst %.al, $(BUILD_DIR)/altlib/%.o, $(SRCS_AL))

all: $(OBJS_AL)

$(BUILD_DIR)/altlib/%.o: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/altlib

clean:
	rm -r $(BUILD_DIR)/altlib
