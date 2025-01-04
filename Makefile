BUILD_DIR=$(abspath build)

.PHONY: compiler arch stdlib altlib test perftest clean

compiler: arch stdlib
	$(MAKE) -C compiler BUILD_DIR=$(BUILD_DIR)

arch:
	$(MAKE) -C arch BUILD_DIR=$(BUILD_DIR)

stdlib:
	$(MAKE) -C stdlib BUILD_DIR=$(BUILD_DIR)

altlib: compiler
	$(MAKE) -C altlib BUILD_DIR=$(BUILD_DIR)

test: compiler altlib
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)

perftest: compiler altlib
	$(MAKE) -C test perf BUILD_DIR=$(BUILD_DIR)

clean:
	rm -r build
