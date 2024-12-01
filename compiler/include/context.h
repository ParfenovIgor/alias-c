#pragma once

#include <vector.h>
#include <typeast.h>
#include <stdbool.h>

struct VariableInfo {
    const char *name;
    struct TypeNode *type;
    int sf_phase;
};

struct TypeInfo {
    const char *name;
    struct TypeNode *type;
};

struct FunctionInfo {
    const char *name_front;
    const char *name_back;
    struct TypeNode *caller_type;
    struct TypeNode *type;
};

struct LabelInfo {
    const char *name;
    const char *name_begin;
    const char *name_end;
    struct TypeNode *type;
    int sf_pos;
};

struct CPContext {
    struct Vector variables;
    struct Vector types;
    struct Vector functions;
    struct Vector block_labels;
    struct Vector loop_labels;
    int sf_pos;

    int function_index;
    int branch_index;
    int data_index;
    int bss_index;
    
    struct Vector test_names;
    bool testing;

    bool header;

    int fd_text;
    int fd_data;
    int fd_bss;

    struct TypeNode *node_void;
    struct TypeNode *node_int;
    struct TypeNode *node_char;
};

struct VariableInfo *context_find_variable      (struct CPContext*, const char*);
struct TypeInfo     *context_find_type          (struct CPContext*, const char*);
struct FunctionInfo *context_find_function      (struct CPContext*, const char*);
struct FunctionInfo *context_find_method        (struct CPContext*, const char*, struct TypeNode*);
struct LabelInfo    *context_find_block_label   (struct CPContext*, const char*);
struct LabelInfo    *context_find_loop_label    (struct CPContext*, const char*);
