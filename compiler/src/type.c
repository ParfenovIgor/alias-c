#include <type.h>
#include <stdlib.h>

struct Type *BuildType(const char *id, int deg) {
    struct Type *type = (struct Type*)_malloc(sizeof(struct Type));
    type->identifier = id;
    type->degree = deg;
    return type;
}

struct Type *CopyType(struct Type *type) {
    struct Type *_type = (struct Type*)_malloc(sizeof(struct Type));
    _type->identifier = type->identifier;
    _type->degree = type->degree;
    return _type;
}
