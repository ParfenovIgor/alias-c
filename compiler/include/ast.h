#pragma once

#include <vector.h>
#include <typeast.h>
#include <stdbool.h>

enum NodeType {
    NodeBlock,
    NodeInclude,
    NodeTest,
    NodeIf,
    NodeWhile,
    NodeFunctionDefinition,
    NodePrototype,
    NodeDefinition,
    NodeTypeDefinition,
    NodeReturn,
    NodeAs,
    NodeAssignment,
    NodeMovement,
    NodeAssumption,
    NodeIdentifier,
    NodeInteger,
    NodeChar,
    NodeString,
    NodeArray,
    NodeStructInstance,
    NodeSizeof,
    NodeFunctionCall,
    NodeDereference,
    NodeIndex,
    NodeGetField,
    NodeAddition,
    NodeSubtraction,
    NodeMultiplication,
    NodeDivision,
    NodeLess,
    NodeEqual,
};

struct Node;
struct Block;
struct Include;
struct Test;
struct If;
struct While;
struct FunctionDefinition;
struct Prototype;
struct Definition;
struct TypeDefinition;
struct Return;
struct As;
struct Assignment;
struct Movement;
struct Assumption;
struct Identifier;
struct Integer;
struct Char;
struct String;
struct Array;
struct StructInstance;
struct Sizeof;
struct Alloc;
struct Free;
struct FunctionCall;
struct Dereference;
struct Index;
struct GetField;
struct Addition;
struct Subtraction;
struct Multiplication;
struct Division;
struct Less;
struct Equal;


struct FunctionSignature {
    struct Vector identifiers;
    struct Vector types;
    struct TypeNode *return_type;
};

struct Node {
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
    void *node_ptr;
    enum NodeType node_type;
};

struct Block {
    struct Vector statement_list;
};

struct Include {
    struct Vector statement_list;
};

struct Test {
    const char *name;
    struct Node *block;
};

struct If {
    struct Vector condition_list;
    struct Vector block_list;
    struct Node *else_block;
};

struct While {
    struct Node *condition;
    struct Node *block;
};

struct FunctionDefinition {
    const char *struct_name;
    const char *name;
    struct FunctionSignature *signature;
    struct Node *block;
    bool external;
};

struct Prototype {
    const char *struct_name;
    const char *name;
    struct FunctionSignature *signature;
};

struct Definition {
    const char *identifier;
    struct Node *value;
};

struct TypeDefinition {
    const char *identifier;
    struct TypeNode *type;
};

struct Return {
    struct Node *expression;
};

struct As {
    struct Node *expression;
    struct TypeNode *type;
};

struct Index {
    struct Node *left, *right;
    bool address;
};

struct GetField {
    struct Node *left;
    const char *field;
    bool address;
};

struct Assignment {
    struct Node *dst, *src;
};

struct Movement {
    struct Node *dst, *src;
};

struct Identifier {
    const char *identifier;
};

struct Integer {
    int value;
};

struct Char {
    int value;
};

struct String {
    const char *value;
};

struct Array {
    struct Vector values;
};

struct StructInstance {
    struct Vector names;
    struct Vector values;
};

struct Sizeof {
    struct TypeNode *type;
};

struct FunctionCall {
    struct Node *function;
    struct Vector arguments;
};

struct Dereference {
    struct Node *expression;
};

struct BinaryOperator {
    struct Node *left, *right;
};
