#pragma once

#include "../include/common.h"
#include "../include/type.h"
#include "../stdlib/include/stdbool.h"

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
    NodeMovementStructure,
    NodeMovementString,
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
struct MovementStructure;
struct MovementString;
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
    struct Node **size_in, **size_out;
    bool **is_const;
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
    const char **function_name;
    int **function_name_index;
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
    const char *name;
    const char **metavariables;
    struct FunctionSignature *signature;
    struct Node *block;
    bool external;
};

struct Prototype {
    const char *name;
    const char **metavariables;
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
};

struct GetField {
    struct Node *left, *right;
};

struct Assignment {
    const char *identifier;
    struct Node *value;
};

struct Movement {
    const char *identifier;
    struct Node *value;
};

struct MovementStructure {
    const char *identifier;
    const char *field;
    struct Node *value;
};

struct MovementString {
    const char *identifier;
    const char *value;
};

struct Assumption {
    const char *identifier;
    struct Node *left, *right;
    struct Node *statement;
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

struct Alloc {
    struct Node *expression;
};

struct Free {
    struct Node *expression;
};

struct FunctionCall {
    const char *identifier;
    const char **metavariable_name;
    struct Node **metavariable_value;
    struct Node **arguments;
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
