ALFLAGS=-i altlib ../../altlib/
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_AL := $(patsubst %.al, $(BUILD_DIR)/test/integration/%, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(EXES_AL)

$(BUILD_DIR)/test/integration/%: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %.o, $@)
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_ARCH) $(OBJS_ALTLIB)
	$@ > $(patsubst %, %.out, $@)
	diff $(patsubst %, %.out, $@) $(patsubst %.al, %.out, $<) || (echo "Test failed"; exit 1)

make_dir:
	mkdir -p $(BUILD_DIR)/test/integration
