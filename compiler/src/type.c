#include <type.h>
#include <context.h>
#include <stdlib.h>
#include <string.h>

struct Type *type_build(const char *id, int deg) {
    struct Type *type = (struct Type*)_malloc(sizeof(struct Type));
    type->identifier = id;
    type->degree = deg;
    return type;
}

struct Type *type_copy(struct Type *type) {
    struct Type *_type = (struct Type*)_malloc(sizeof(struct Type));
    _type->identifier = type->identifier;
    _type->degree = type->degree;
    return _type;
}

bool type_equal(struct Type *type1, struct Type *type2) {
    return (!_strcmp(type1->identifier, type2->identifier) && type1->degree == type2->degree);
}

int type_size(struct Type *type, struct CPContext *context, bool aligned) {
    if (type->degree != 0) {
        return 8;
    }
    if (_strcmp(type->identifier, "int") == 0) {
        return 8;
    }
    if (_strcmp(type->identifier, "char") == 0) {
        if (aligned) {
            return 8;
        }
        else {
            return 1;
        }
    }
    else {
        struct StructInfo *_struct = context_find_struct(context, type->identifier);
        return _struct->size;
    }
}

bool type_is_int(struct Type *type, struct CPContext *context) {
    if (type->degree != 0) {
        return false;
    }
    if (_strcmp(type->identifier, "int") == 0) {
        return true;
    }
    if (_strcmp(type->identifier, "char") == 0) {
        return true;
    }
    return false;
}
