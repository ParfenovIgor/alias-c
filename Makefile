BUILD_DIR=$(abspath build)

ASFLAGS=-f elf64
CFLAGS=-g -fno-stack-protector
LDFLAGS=-z noexecstack

.PHONY: compiler stdlib altlib test clean

compiler: stdlib
	$(MAKE) -C compiler BUILD_DIR=$(BUILD_DIR)

stdlib:
	$(MAKE) -C stdlib BUILD_DIR=$(BUILD_DIR)

altlib: compiler
	$(MAKE) -C altlib BUILD_DIR=$(BUILD_DIR)

test: compiler altlib
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)

clean:
	rm -r build
