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
