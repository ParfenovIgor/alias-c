#include <type.h>
#include <stdlib.h>
#include <string.h>

bool type_equal(struct TypeNode *n1, struct TypeNode *n2, struct CPContext *context) {
    int sum_degree1 = 0, sum_degree2 = 0;
    while (n1->node_type == TypeNodeIdentifier) {
        sum_degree1 += n1->degree;
        struct TypeIdentifier *_n1 = n1->node_ptr;
        n1 = context_find_type(context, _n1->identifier)->type;
    }
    while (n2->node_type == TypeNodeIdentifier) {
        sum_degree1 += n2->degree;
        struct TypeIdentifier *_n2 = n2->node_ptr;
        n2 = context_find_type(context, _n2->identifier)->type;
    }

    if (n1->node_type != n2->node_type ||
        n1->degree + sum_degree1 != n2->degree + sum_degree2) {
        return false;
    }
    if (n1->node_type == TypeNodeInt) return true;
    if (n1->node_type == TypeNodeChar) return true;
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

struct TypeNode *type_get_function(struct TypeNode *n, struct CPContext *context) {
    int sum_degree = 0;
    while (n->node_type == TypeNodeIdentifier) {
        sum_degree += n->degree;
        struct TypeIdentifier *_n = n->node_ptr;
        n = context_find_type(context, _n->identifier)->type;
    }

    if (n->node_type == TypeNodeFunction) {
        return n;
    }
    else {
        return NULL;
    }
}

int type_size(struct TypeNode *n, struct CPContext *context) {
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
