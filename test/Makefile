BUILD_DIR=$(abspath build)

.PHONY: algo unit integration

all: algo unit integration

algo:
	$(MAKE) -C algo BUILD_DIR=$(BUILD_DIR)

unit:
	$(MAKE) -C unit BUILD_DIR=$(BUILD_DIR)

integration:
	$(MAKE) -C integration BUILD_DIR=$(BUILD_DIR)
