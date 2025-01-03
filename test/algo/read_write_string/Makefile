ALFLAGS=-i altlib ../../../altlib/
LDFLAGS=-z noexecstack

TESTS_IN := $(wildcard in/*)
TESTS_OUT := $(wildcard out/*)
TESTS_OUT2 := $(patsubst out/%, $(WORKING_DIR)/out/%, $(TESTS_OUT))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(WORKING_DIR)/main $(TESTS_OUT2)

$(WORKING_DIR)/main: main.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %.o, $@)
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_ARCH) $(OBJS_ALTLIB)

$(WORKING_DIR)/out/%: in/% $(WORKING_DIR)/main
	$(WORKING_DIR)/main < $< > $@
	diff $@ $(patsubst in/%, out/%, $<) || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p $(WORKING_DIR)/out
