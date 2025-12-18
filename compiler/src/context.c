#include <ast.h>
#include <type.h>
#include <context.h>
#include <string.h>

struct CPContext *context_init() {
    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variables = vnew();
    context->global_variables = vnew();
    context->types = vnew();
    context->functions = vnew();
    context->block_labels = vnew();
    context->loop_labels = vnew();
    context->sf_pos = 0;
    context->function_index = 0;
    context->branch_index = 0;
    context->data_index = 0;
    context->bss_index = 0;
    context->test_names = vnew();
    context->testing = false;
    context->header = false;

    {
        context->node_void = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeVoid *_type = (struct TypeVoid*)_malloc(sizeof(struct TypeVoid));
        context->node_void->node_ptr = _type;
        context->node_void->node_type = TypeNodeVoid;
        context->node_void->degree = 0;
        context->node_void->size = -1;
    }
    {
        context->node_int = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeInt *_type = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
        context->node_int->node_ptr = _type;
        context->node_int->node_type = TypeNodeInt;
        context->node_int->degree = 0;
        context->node_int->size = -1;
    }
    {
        context->node_char = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeChar *_type = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
        context->node_char->node_ptr = _type;
        context->node_char->node_type = TypeNodeChar;
        context->node_char->degree = 0;
        context->node_char->size = -1;
    }
    {
        context->node_allocator = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeIdentifier *_type = (struct TypeIdentifier*)_malloc(sizeof(struct TypeIdentifier));
        context->node_allocator->node_ptr = _type;
        context->node_allocator->node_type = TypeNodeIdentifier;
        context->node_allocator->degree = 1;
        context->node_allocator->size = -1;
        _type->identifier = _strdup("TestAllocator");
    }
    return context;
}

struct GlobalVariableInfo *context_find_global_variable(struct CPContext *context, const char *identifier) {
    int sz = vsize(&context->global_variables);
    for (int i = sz - 1; i >= 0; i--) {
        struct GlobalVariableInfo *global_var_info = context->global_variables.ptr[i];
        if (_strcmp(global_var_info->name, identifier) == 0) {
            return global_var_info;
        }
    }
    return NULL;
}

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
        if (_strcmp(function_info->name_front, identifier) == 0 && !function_info->caller_type) {
            return function_info;
        }
    }
    return NULL;
}

struct FunctionInfo *context_find_method(struct CPContext *context, const char *identifier, struct TypeNode *type) {
    int sz = vsize(&context->functions);
    for (int i = sz - 1; i >= 0; i--) {
        struct FunctionInfo *function_info = context->functions.ptr[i];
        if (_strcmp(function_info->name_front, identifier) == 0 && function_info->caller_type && type_equal(function_info->caller_type, type, context)) {
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
