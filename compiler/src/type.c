#include <type.h>
#include <memory.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>

bool type_check(struct TypeNode *n, struct CPContext *context) {
    if (n->node_type == TypeNodeVoid) return true;
    if (n->node_type == TypeNodeInt) return true;
    if (n->node_type == TypeNodeChar) return true;
    if (n->node_type == TypeNodeStruct) {
        struct TypeStruct *_n = n->node_ptr;
        int sz = vsize(&_n->names);
        for (int i = 0; i < sz; i++) {
            if (!type_check(_n->types.ptr[i], context)) return false;
        }
        return true;
    }
    if (n->node_type == TypeNodeFunction) {
        struct TypeFunction *_n = n->node_ptr;
        int sz = vsize(&_n->types);
        for (int i = 0; i < sz; i++) {
            if (!type_check(_n->types.ptr[i], context)) return false;
        }
        return type_check(_n->return_type, context);
    }
    if (n->node_type == TypeNodeIdentifier) {
        struct TypeIdentifier *_n = n->node_ptr;
        struct TypeInfo *info = context_find_type(context, _n->identifier);
        if (!info) return false;
        return type_check(info->type, context);
    }
}

bool type_equal(struct TypeNode *n1, struct TypeNode *n2, struct CPContext *context) {
    int sum_degree1 = 0, sum_degree2 = 0;
    while (n1->node_type == TypeNodeIdentifier) {
        sum_degree1 += n1->degree;
        struct TypeIdentifier *_n1 = n1->node_ptr;
        struct TypeInfo *info = context_find_type(context, _n1->identifier);
        if (!info) {
            _panic("Type identifier not found");
        }
        n1 = info->type;
    }
    while (n2->node_type == TypeNodeIdentifier) {
        sum_degree2 += n2->degree;
        struct TypeIdentifier *_n2 = n2->node_ptr;
        struct TypeInfo *info = context_find_type(context, _n2->identifier);
        if (!info) {
            _panic("Type identifier not found");
        }
        n2 = info->type;
    }

    if (n1->node_type != n2->node_type ||
        n1->degree + sum_degree1 != n2->degree + sum_degree2) {
        return false;
    }
    if (n1->node_type == TypeNodeVoid) return n2->node_type == TypeNodeVoid;
    if (n1->node_type == TypeNodeInt) return n2->node_type == TypeNodeInt;
    if (n1->node_type == TypeNodeChar) return n2->node_type == TypeNodeChar;
    if (n1->node_type == TypeNodeStruct) {
        struct TypeStruct *_n1 = n1->node_ptr;
        struct TypeStruct *_n2 = n2->node_ptr;
        int sz = vsize(&_n1->names);
        for (int i = 0; i < sz; i++) {
            if (_strcmp(_n1->names.ptr[i], _n2->names.ptr[i]) ||
                !type_equal(_n1->types.ptr[i], _n2->types.ptr[i], context)) {
                return false;
            }
        }
        return true;
    }
    if (n1->node_type == TypeNodeFunction) {
        struct TypeFunction *_n1 = n1->node_ptr;
        struct TypeFunction *_n2 = n2->node_ptr;
        int sz = vsize(&_n1->types);
        for (int i = 0; i < sz; i++) {
            if (!type_equal(_n1->types.ptr[i], _n2->types.ptr[i], context)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

struct TypeNode *type_get_struct_pointer(struct TypeNode *n, struct CPContext *context) {
    int sum_degree = 0;
    while (n->node_type == TypeNodeIdentifier) {
        sum_degree += n->degree;
        struct TypeIdentifier *_n = n->node_ptr;
        n = context_find_type(context, _n->identifier)->type;
    }

    if (n->node_type == TypeNodeStruct && n->degree + sum_degree == 1) {
        struct TypeNode *type = type_copy_node(n);
        type->degree = 1;
        return type;
    }
    else {
        return NULL;
    }
}

struct TypeNode *type_get_function(struct TypeNode *n, struct CPContext *context) {
    int sum_degree = 0;
    while (n->node_type == TypeNodeIdentifier) {
        sum_degree += n->degree;
        struct TypeIdentifier *_n = n->node_ptr;
        n = context_find_type(context, _n->identifier)->type;
    }

    if (n->node_type == TypeNodeFunction && n->degree + sum_degree == 0) {
        return n;
    }
    else {
        return NULL;
    }
}

int type_size(struct TypeNode *n, struct CPContext *context) {
    if (n->degree > 0) return 8;
    if (n->node_type == TypeNodeVoid) return 0;
    if (n->node_type == TypeNodeInt) return 8;
    if (n->node_type == TypeNodeChar) return 1;
    if (n->node_type == TypeNodeStruct) {
        struct TypeStruct *_n = n->node_ptr;
        int res = 0;
        int sz = vsize(&_n->names);
        for (int i = 0; i < sz; i++) {
            res += type_size(_n->types.ptr[i], context);
        }
        return res;
    }
    if (n->node_type == TypeNodeFunction) return 8;
    if (n->node_type == TypeNodeIdentifier) {
        struct TypeIdentifier *_n = n->node_ptr;
        return type_size(context_find_type(context, _n->identifier)->type, context);
    }
    return 0;
}

struct TypeNode *type_copy_node(struct TypeNode *n1) {
    struct TypeNode *n2 = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    n2->node_ptr = n1->node_ptr;
    n2->node_type = n1->node_type;
    n2->degree = n1->degree;
    return n2;
}

int type_mangle_helper(struct TypeNode *n, struct CPContext *context, char *buffer, int pos) {
    int sum_degree = 0;
    while (n->node_type == TypeNodeIdentifier) {
        sum_degree += n->degree;
        struct TypeIdentifier *_n = n->node_ptr;
        struct TypeInfo *info = context_find_type(context, _n->identifier);
        if (!info) {
            _panic("Type identifier not found");
        }
        n = context_find_type(context, _n->identifier)->type;
    }
    const char *ptr_str = _itoa(sum_degree + n->degree);
    _strcpy(buffer + pos, ptr_str);
    pos += _strlen(ptr_str);

    if (n->node_type == TypeNodeVoid) {
        buffer[pos++] = 'V';
        return pos;
    }
    if (n->node_type == TypeNodeInt) {
        buffer[pos++] = 'I';
        return pos;
    }
    if (n->node_type == TypeNodeChar) {
        buffer[pos++] = 'C';
        return pos;
    }
    if (n->node_type == TypeNodeStruct) {
        buffer[pos++] = 'S';
        struct TypeStruct *_n = n->node_ptr;

        int sz = vsize(&_n->names);
        const char *ptr_str = _itoa(sz);
        int len = _strlen(ptr_str);
        for (int i = 0; i < len; i++) buffer[pos++] = ptr_str[i];

        for (int i = 0; i < sz; i++) {
            buffer[pos++] = '_';
            _strcpy(buffer + pos, _n->names.ptr[i]);
            pos += _strlen(_n->names.ptr[i]);
            buffer[pos++] = '_';
            pos = type_mangle_helper(_n->types.ptr[i], context, buffer, pos);
        }
        return pos;
    }
    if (n->node_type == TypeNodeFunction) {
        buffer[pos++] = 'F';
        struct TypeFunction *_n = n->node_ptr;

        int sz = vsize(&_n->types);
        const char *ptr_str = _itoa(sz);
        int len = _strlen(ptr_str);
        for (int i = 0; i < len; i++) buffer[pos++] = ptr_str[i];

        for (int i = 0; i < sz; i++) {
            buffer[pos++] = '_';
            pos = type_mangle_helper(_n->types.ptr[i], context, buffer, pos);
        }
        buffer[pos++] = '_';
        pos = type_mangle_helper(_n->return_type, context, buffer, pos);
        return pos;
    }
}

const char *type_mangle(struct TypeNode *type, struct CPContext *context) {
    char *buffer = (char*)_malloc(1024 * sizeof(char));
    _memset(buffer, 0, 1024 * sizeof(char));
    buffer[0] = '_';
    buffer[1] = 'T';
    type_mangle_helper(type, context, buffer, 2);
    return buffer;
}
