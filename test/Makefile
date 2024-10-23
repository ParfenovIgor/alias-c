BUILD_DIR=$(abspath build)

.PHONY: unit integration

all: unit integration

unit:
	$(MAKE) -C unit BUILD_DIR=$(BUILD_DIR)

integration:
	$(MAKE) -C integration BUILD_DIR=$(BUILD_DIR)

clean:
	rm -r build
