.PHONY: algo integration perf unit

all: unit integration algo

algo:
	$(MAKE) -C algo        SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

integration:
	$(MAKE) -C integration SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

perf:
	$(MAKE) -C perf        SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

unit:
	$(MAKE) -C unit        SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)
