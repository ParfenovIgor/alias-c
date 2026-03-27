ALFLAGS=-i altlib ../../../altlib/
LDFLAGS=-z noexecstack

TESTS_IN := $(wildcard in/*)
TESTS_OUT := $(wildcard out/*)
TESTS_OUT_x86_64 := $(patsubst out/%, $(WORKING_DIR)/x86_64/out/%, $(TESTS_OUT))
TESTS_OUT_C := $(patsubst out/%, $(WORKING_DIR)/c/out/%, $(TESTS_OUT))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(WORKING_DIR)/x86_64/main $(TESTS_OUT_x86_64) \
     $(WORKING_DIR)/c/main $(TESTS_OUT_C)

$(WORKING_DIR)/x86_64/main: main.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %.o, $@)
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_ARCH) $(OBJS_ALTLIB)

$(WORKING_DIR)/x86_64/out/%: in/% $(WORKING_DIR)/x86_64/main
	$(WORKING_DIR)/x86_64/main < $< > $@
	diff $@ $(patsubst in/%, out/%, $<) || (echo "Test failed"; exit 1)

$(WORKING_DIR)/c/main: main.al make_dir
	$(BUILD_DIR)/calias -a -b c $(ALFLAGS) $< -o $(patsubst %, %.c, $@)
	gcc -fno-stack-protector -g -c $(patsubst %, %.c, $@) -o $(patsubst %, %.o, $@) -no-pie
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_ARCH) $(OBJS_ALTLIB)

$(WORKING_DIR)/c/out/%: in/% $(WORKING_DIR)/c/main
	$(WORKING_DIR)/c/main < $< > $@
	diff $@ $(patsubst in/%, out/%, $<) || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p $(WORKING_DIR)/x86_64/out
	mkdir -p $(WORKING_DIR)/c/out
