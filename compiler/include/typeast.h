#pragma once

#include <stdbool.h>
#include <vector.h>

enum TypeNodeType {
    TypeNodeVoid,
    TypeNodeInt,
    TypeNodeChar,
    TypeNodeStruct,
    TypeNodeFunction,
    TypeNodeIdentifier,
};

struct TypeNode;
struct TypeVoid;
struct TypeInt;
struct TypeChar;
struct TypeStruct;
struct TypeFunction;
struct TypeIdentifier;


struct TypeNode {
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
    void *node_ptr;
    enum TypeNodeType node_type;
    int degree;
    int size;
};

struct TypeVoid {

};

struct TypeInt {

};

struct TypeChar {

};

struct TypeStruct {
    struct Vector names;
    struct Vector types;
};

struct TypeFunction {
    struct Vector types;
    struct TypeNode *return_type;
    bool propagate_allocator;
};

struct TypeIdentifier {
    const char *identifier;
};

struct TypeNode *create_type_node(void *ptr, enum TypeNodeType type, int degree, int size);
struct TypeNode *create_type_void(int degree);
struct TypeNode *create_type_int(int degree);
struct TypeNode *create_type_char(int degree);
struct TypeNode *create_type_struct(int degree, struct Vector names, struct Vector types);
struct TypeNode *create_type_function(int degree, struct Vector types, struct TypeNode *return_type);
struct TypeNode *create_type_identifier(int degree, const char *identifier);
