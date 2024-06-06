#include "../include/type.h"
#include "../stdlib/include/stdlib.h"

struct Type *BuildType(const char *id, int deg) {
    struct Type *type = (struct Type*)_malloc(sizeof(struct Type));
    type->identifier = id;
    type->degree = deg;
    return type;
}
