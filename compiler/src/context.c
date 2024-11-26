#include <ast.h>
#include <context.h>
#include <string.h>

struct VariableInfo *context_find_variable(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->variables);
    for (int i = sz - 1; i >= 0; i--) {
        struct VariableInfo *var_info = context->variables.ptr[i];
        if (_strcmp(var_info->name, identifier) == 0) {
            return var_info;
        }
    }
    return NULL;
}

struct TypeInfo *context_find_type(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->types);
    for (int i = sz - 1; i >= 0; i--) {
        struct TypeInfo *type_info = context->types.ptr[i];
        if (_strcmp(type_info->name, identifier) == 0) {
            return type_info;
        }
    }
    return NULL;
}

struct FunctionInfo *context_find_function(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->functions);
    for (int i = sz - 1; i >= 0; i--) {
        struct FunctionInfo *function_info = context->functions.ptr[i];
        if (_strcmp(function_info->name_front, identifier) == 0) {
            return function_info;
        }
    }
    return NULL;
}

struct LabelInfo *context_find_block_label(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->block_labels);
    for (int i = sz - 1; i >= 0; i--) {
        struct LabelInfo *label_info = context->block_labels.ptr[i];
        if (label_info->name && identifier && _strcmp(label_info->name, identifier) == 0 || !identifier) {
            return label_info;
        }
    }
    return NULL;
}

struct LabelInfo *context_find_loop_label(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->loop_labels);
    for (int i = sz - 1; i >= 0; i--) {
        struct LabelInfo *label_info = context->loop_labels.ptr[i];
        if (label_info->name && identifier && _strcmp(label_info->name, identifier) == 0 || !identifier) {
            return label_info;
        }
    }
    return NULL;
}
