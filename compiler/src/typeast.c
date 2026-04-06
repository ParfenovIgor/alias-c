#include <typeast.h>
#include <stdlib.h>

#define Alloc(T, x) struct T *x = (struct T*)_malloc(sizeof(struct T));

struct TypeNode *create_type_node(void *ptr, enum TypeNodeType type, int degree) {
    Alloc(TypeNode, node);
    node->node_ptr = ptr;
    node->node_type = type;
    node->degree = degree;
    node->size = 1;
    return node;
}

struct TypeNode *create_type_void(int degree) {
    Alloc(TypeVoid, this);
    return create_type_node(this, TypeNodeVoid, degree);
}

struct TypeNode *create_type_int(int degree) {
    Alloc(TypeInt, this);
    return create_type_node(this, TypeNodeInt, degree);
}

struct TypeNode *create_type_char(int degree) {
    Alloc(TypeChar, this);
    return create_type_node(this, TypeNodeChar, degree);
}

struct TypeNode *create_type_struct(int degree, struct Vector names, struct Vector types) {
    Alloc(TypeStruct, this);
    this->names = names;
    this->types = types;
    return create_type_node(this, TypeNodeStruct, degree);
}

struct TypeNode *create_type_function(int degree, struct Vector types, struct TypeNode *return_type) {
    Alloc(TypeFunction, this);
    this->types = types;
    this->return_type = return_type;
    return create_type_node(this, TypeNodeFunction, degree);
}

struct TypeNode *create_type_identifier(int degree, const char *identifier) {
    Alloc(TypeIdentifier, this);
    this->identifier = identifier;
    return create_type_node(this, TypeNodeIdentifier, degree);
}
