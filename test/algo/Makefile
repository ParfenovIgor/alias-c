ALFLAGS=-i altlib ../../altlib/
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_AL := $(patsubst %.al, $(BUILD_DIR)/test/algo/%, $(SRCS_AL))

OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)

all: $(EXES_AL)

$(BUILD_DIR)/test/algo/%: %.al make_dir
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %.o, $@)
	ld $(LDFLAGS) -o $@ $(patsubst %, %.o, $@) $(OBJS_STDLIB) $(OBJS_ALTLIB)

make_dir:
	mkdir -p $(BUILD_DIR)/test/algo

clean:
	rm -r $(BUILD_DIR)/test
