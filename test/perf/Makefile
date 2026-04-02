ALFLAGS=-i altlib ../../altlib/
CFLAGS=-g -fno-stack-protector -I include -I ../../stdlib/include
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_x86_64 := $(patsubst %.al, $(BUILD_DIR)/test/perf/x86_64/%, $(SRCS_AL))
EXES_C := $(patsubst %.al, $(BUILD_DIR)/test/perf/c/%, $(SRCS_AL))
EXES_GCC := $(patsubst %.al, $(BUILD_DIR)/test/perf/gcc/%, $(SRCS_AL))
REPORTS := $(patsubst %.al, $(BUILD_DIR)/test/perf/%, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)
OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(REPORTS)

$(BUILD_DIR)/test/perf/%: %.al make_dir
	echo $(patsubst %.al, %, $<) > $@

	echo "calias x86_64" >> $@
	$(eval $@_DIR = $(patsubst %.al, $(BUILD_DIR)/test/perf/x86_64/%, $<))
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $($@_DIR).o
	ld $(LDFLAGS) -o $($@_DIR) $($@_DIR).o $(OBJS_ARCH) $(OBJS_ALTLIB)
	time -p $($@_DIR) 2> $($@_DIR).time || (echo "Test failed"; exit 1)
	cat $($@_DIR).time | grep real | awk '{print $$2}' >> $@

	echo "calias C" >> $@
	$(eval $@_DIR = $(patsubst %.al, $(BUILD_DIR)/test/perf/c/%, $<))
	$(BUILD_DIR)/calias -a -b c $(ALFLAGS) $< -o $($@_DIR).c
	gcc -fno-stack-protector -O2 -g -c $($@_DIR).c -o $($@_DIR).o -no-pie
	ld $(LDFLAGS) -o $($@_DIR) $($@_DIR).o $(OBJS_ARCH) $(OBJS_ALTLIB)
	time -p $($@_DIR) 2> $($@_DIR).time || (echo "Test failed"; exit 1)
	cat $($@_DIR).time | grep real | awk '{print $$2}' >> $@

	echo "gcc" >> $@
	$(eval $@_DIR = $(patsubst %.al, $(BUILD_DIR)/test/perf/gcc/%, $<))
	gcc $(CFLAGS) -c -O2 $(patsubst %.al, %.c, $<) -o $($@_DIR).o
	gcc $(LDFLAGS) -o $($@_DIR) $($@_DIR).o $(OBJS_ARCH) $(OBJS_STDLIB)
	time -p $($@_DIR) 2> $($@_DIR).time || (echo "Test failed"; exit 1)
	cat $($@_DIR).time | grep real | awk '{print $$2}' >> $@

make_dir:
	mkdir -p $(BUILD_DIR)/test/perf/x86_64
	mkdir -p $(BUILD_DIR)/test/perf/c
	mkdir -p $(BUILD_DIR)/test/perf/gcc
