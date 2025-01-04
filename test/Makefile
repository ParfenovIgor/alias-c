BUILD_DIR=$(abspath build)

.PHONY: algo integration perf unit

all: algo integration unit

algo:
	$(MAKE) -C algo BUILD_DIR=$(BUILD_DIR)

integration:
	$(MAKE) -C integration BUILD_DIR=$(BUILD_DIR)

perf:
	$(MAKE) -C perf BUILD_DIR=$(BUILD_DIR)

unit:
	$(MAKE) -C unit BUILD_DIR=$(BUILD_DIR)
