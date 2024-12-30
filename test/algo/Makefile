ALFLAGS=-i altlib ../../altlib/
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard */main.al)
DIRS := $(patsubst %/main.al, $(BUILD_DIR)/test/algo/%, $(SRCS_AL))

all: $(DIRS)

$(BUILD_DIR)/test/algo/%: % make_dir
	$(MAKE) -C $< BUILD_DIR=$(BUILD_DIR) WORKING_DIR=$(BUILD_DIR)/test/algo/$<

make_dir:
	mkdir -p $(BUILD_DIR)/test/algo

clean:
	rm -r $(BUILD_DIR)/test
