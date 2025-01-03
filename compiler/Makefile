CFLAGS=-g -fno-stack-protector -I include -I ../stdlib/include
LDFLAGS=-z noexecstack

SRCS_C := $(wildcard src/*.c)
OBJS_C := $(patsubst src/%.c, $(BUILD_DIR)/compiler/%.o, $(SRCS_C))

OBJS_ARCH := $(wildcard $(BUILD_DIR)/arch/*.o)
OBJS_STDLIB := $(wildcard $(BUILD_DIR)/stdlib/*.o)

all: $(OBJS_C)
	gcc -g -fno-stack-protector -I include -c httpd/httpd.c -o $(BUILD_DIR)/compiler/httpd.o
	gcc $(LDFLAGS) -o $(BUILD_DIR)/calias $(OBJS_C) $(BUILD_DIR)/compiler/httpd.o $(OBJS_ARCH) $(OBJS_STDLIB)

$(BUILD_DIR)/compiler/%.o: src/%.c make_dir
	gcc $(CFLAGS) -c $< -o $@

make_dir:
	mkdir -p $(BUILD_DIR)/compiler
