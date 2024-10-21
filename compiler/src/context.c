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

struct StructInfo *context_find_struct(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->structs);
    for (int i = 0; i < sz; i++) {
        struct StructInfo *_struct = context->structs.ptr[i];
        if (_strcmp(_struct->name, identifier) == 0) {
            return context->structs.ptr[i];
        }
    }
    return NULL;
}
