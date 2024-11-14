#pragma once

#include <vector.h>

enum TypeNodeType {
    TypeNodeInt,
    TypeNodeChar,
    TypeNodeStruct,
    TypeNodeFunction,
    TypeNodeIdentifier,
};

struct TypeNode;
struct TypeInt;
struct TypeChar;
struct TypeStruct;
struct TypeFunction;
struct TypeIdentifier;


struct TypeNode {
    void *node_ptr;
    enum TypeNodeType node_type;
    int degree;
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
};

struct TypeIdentifier {
    const char *identifier;
};
