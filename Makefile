SOURCE_DIR=$(abspath .)
BUILD_DIR=$(abspath build)

.PHONY: compiler arch stdlib altlib test perftest clean

all: compiler arch stdlib altlib test perftest

compiler: arch stdlib
	$(MAKE) -C compiler  SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

arch:
	$(MAKE) -C arch		 SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

stdlib:
	$(MAKE) -C stdlib	 SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

altlib: compiler
	$(MAKE) -C altlib    SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

test: compiler altlib
	$(MAKE) -C test      SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

perftest: compiler altlib
	$(MAKE) -C test perf SOURCE_DIR=$(SOURCE_DIR) BUILD_DIR=$(BUILD_DIR)

clean:
	rm -r build
