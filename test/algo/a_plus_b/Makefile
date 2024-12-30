ALFLAGS=-i altlib ../../../altlib/
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_AL := $(patsubst %.al, $(BUILD_DIR)/test/integration/%, $(SRCS_AL))

TESTS_IN := $(wildcard in/*)
TESTS_OUT := $(wildcard out/*)
TESTS_OUT2 := $(patsubst out/%, $(WORKING_DIR)/out/%, $(TESTS_OUT))

OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(WORKING_DIR)/main $(TESTS_OUT2)

$(WORKING_DIR)/main: main.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %.o, $@)
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_STDLIB) $(OBJS_ALTLIB)

$(WORKING_DIR)/out/%: in/% $(WORKING_DIR)/main
	$(WORKING_DIR)/main < $< > $@
	diff $@ $(patsubst in/%, out/%, $<) || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p $(WORKING_DIR)/out

clean:
	rm -r $(BUILD_DIR)
