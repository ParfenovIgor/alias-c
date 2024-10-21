#pragma once

#include <ast.h>
#include <stdbool.h>

struct CPContext;

struct Type {
    const char *identifier;
    int degree;
};

struct Type *type_build(const char *id, int deg);
struct Type *type_copy(struct Type *type);
bool type_equal(struct Type *type1, struct Type *type2);
int type_size(struct Type *type, struct CPContext *context, bool aligned);
bool type_is_int(struct Type *type, struct CPContext *context);
