#pragma once

#include <ast.h>
#include <stdbool.h>

struct CPContext;

struct Type {
    const char *identifier;
    int degree;
};

struct Type *type_build(const char *id, int deg);
struct Type *type_copy(struct Type*);
bool type_equal(struct Type*, struct Type*);
int type_size(struct Type*, struct CPContext*, bool aligned);
bool type_is_int(struct Type*, struct CPContext*);
