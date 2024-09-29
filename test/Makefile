LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard src/*.al)
OBJS_AL := $(patsubst src/%.al, $(BUILD_DIR)/test/%.o, $(SRCS_AL))

OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(OBJS_AL)
	ld $(LDFLAGS) -o $(BUILD_DIR)/test/testal $(OBJS_AL) $(OBJS_STDLIB) $(OBJS_ALTLIB)
	$(BUILD_DIR)/test/testal > $(BUILD_DIR)/test/output
	diff $(BUILD_DIR)/test/output output || (echo "Test failed"; exit 1)

$(BUILD_DIR)/test/%.o: src/%.al make_dir
	$(BUILD_DIR)/calias -a $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/test

clean:
	rm -r $(BUILD_DIR)/test
