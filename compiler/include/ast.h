#pragma once

#include <common.h>
#include <type.h>
#include <stdbool.h>

enum NodeType {
    NodeBlock,
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
    NodeChar,
    NodeString,
    NodeArray,
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
struct Char;
struct String;
struct Array;
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
    int data_index;
    int bss_index;
    int fd_text;
    int fd_data;
    int fd_bss;
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

struct Char {
    int value;
};

struct String {
    const char *value;
};

struct Array {
    struct Node **values;
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

struct BinaryOperator {
    struct Node *left, *right;
};
