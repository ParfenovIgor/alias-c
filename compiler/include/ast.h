#pragma once

#include <common.h>
#include <type.h>
#include <stdbool.h>

enum NodeType {
    NodeBlock,
    NodeAsm,
    NodeIf,
    NodeWhile,
    NodeFunctionDefinition,
    NodePrototype,
    NodeStructDefinition,
    NodeDefinition,
    NodeReturn,
    NodeAs,
    NodeAssignment,
    NodeMovement,
    NodeAssumption,
    NodeIdentifier,
    NodeInteger,
    NodeSizeof,
    NodeAlloc,
    NodeFree,
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
struct Asm;
struct If;
struct While;
struct FunctionDefinition;
struct Prototype;
struct StructDefinition;
struct Definition;
struct Return;
struct As;
struct Assignment;
struct Movement;
struct Assumption;
struct Identifier;
struct Integer;
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
    const char **identifiers;
    struct Type **types;
    struct Type *return_type;
};

struct Struct {
    const char *name;
    const char **identifiers;
    struct Type **types;
};

struct CPContext {
    const char **variable_local_name;
    struct Type **variable_local_type;
    const char **variable_argument_name;
    struct Type **variable_argument_type;
    const char **function_name_front;
    const char **function_name_back;
    struct FunctionSignature **function_signature;
    struct Struct **structs;
    int function_index;
    int branch_index;
    int outputFileDescriptor;
};

struct Node {
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
    void *node_ptr;
    enum NodeType node_type;
};

struct Block {
    struct Node **statement_list;
};

struct Asm {
    const char *code;
};

struct If {
    struct Node **condition_list;
    struct Node **block_list;
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

struct StructDefinition {
    const char *name;
    const char **identifiers;
    struct Type **types;
};

struct Definition {
    const char *identifier;
    struct Type *type;
};

struct Return {
    struct Node *expression;
};

struct As {
    struct Node *expression;
    struct Type *type;
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

struct Sizeof {
    struct Type *type;
};

struct FunctionCall {
    const char *identifier;
    struct Node **arguments;
    struct Node *caller;
};

struct Dereference {
    struct Node *expression;
};

struct Addition {
    struct Node *left, *right;
};

struct Subtraction {
    struct Node *left, *right;
};

struct Multiplication {
    struct Node *left, *right;
};

struct Division {
    struct Node *left, *right;
};

struct Less {
    struct Node *left, *right;
};

struct Equal {
    struct Node *left, *right;
};
