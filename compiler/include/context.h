#pragma once

#include <vector.h>
#include <type.h>

struct VariableInfo {
    const char *name;
    struct Type *type;
    int sf_phase;
};

struct StructInfo {
    const char *name;
    struct Vector variables;
    int size;
};

struct CPContext {
    struct Vector variables;
    int sf_pos;

    struct Vector function_name_front;
    struct Vector function_name_back;
    struct Vector function_signature;

    struct Vector structs;

    int function_index;
    int branch_index;
    int data_index;
    int bss_index;

    int fd_text;
    int fd_data;
    int fd_bss;
};

struct VariableInfo *context_find_variable(struct CPContext*, const char*);
struct StructInfo *context_find_struct(struct CPContext*, const char*);
