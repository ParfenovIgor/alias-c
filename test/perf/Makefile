ALFLAGS=-i altlib ../../altlib/
CFLAGS=-g -fno-stack-protector -I include -I ../../stdlib/include
LDFLAGS=-z noexecstack

SRCS_AL := $(wildcard *.al)
EXES_AL := $(patsubst %.al, $(BUILD_DIR)/test/perf/%, $(SRCS_AL))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_ALTLIB := $(wildcard $(BUILD_DIR)/altlib/*.o)
OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(EXES_AL)

$(BUILD_DIR)/test/perf/%: %.al make_dir
	echo $(patsubst %.al, %, $<) > $(patsubst %, %_report, $@)

	echo "calias" >> $(patsubst %, %_report, $@)
	$(BUILD_DIR)/calias -a $(ALFLAGS) $< -o $(patsubst %, %1.o, $@)
	ld $(LDFLAGS) -o $(patsubst %, %1, $@) $(patsubst %, %1.o, $@) $(OBJS_ARCH) $(OBJS_ALTLIB)
	{ time -p $(patsubst %, %1, $@) ; } 2>&1 | grep real | awk '{print $$2}' >> $(patsubst %, %_report, $@)

	echo "gcc" >> $(patsubst %, %_report, $@)
	gcc $(CFLAGS) -c $(patsubst %.al, %.c, $<) -o $(patsubst %, %2.o, $@)
	gcc $(LDFLAGS) -o $(patsubst %, %2, $@) $(patsubst %, %2.o, $@) $(OBJS_ARCH) $(OBJS_STDLIB)
	{ time -p $(patsubst %, %2, $@) ; } 2>&1 | grep real | awk '{print $$2}' >> $(patsubst %, %_report, $@)

make_dir:
	mkdir -p $(BUILD_DIR)/test/perf
