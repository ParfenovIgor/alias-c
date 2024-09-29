SRCS_AL := $(wildcard src/*.al)
OBJS_AL := $(patsubst src/%.al, $(BUILD_DIR)/altlib/%.o, $(SRCS_AL))

all: $(OBJS_AL)

$(BUILD_DIR)/altlib/%.o: src/%.al make_dir
	$(BUILD_DIR)/calias -a $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/altlib

clean:
	rm -r $(BUILD_DIR)/altlib
