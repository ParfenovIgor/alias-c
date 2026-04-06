#pragma once

#include <vector.h>
#include <typeast.h>
#include <stdbool.h>

enum NodeType {
    NodeModule,
    NodeBlock,
    NodeInclude,
    NodeTest,
    NodeIf,
    NodeWhile,
    NodeFunctionDefinition,
    NodePrototype,
    NodeGlobalDefinition,
    NodeDefinition,
    NodeTypeDefinition,
    NodeReturn,
    NodeBreak,
    NodeContinue,
    NodeAs,
    NodeAssignment,
    NodeMovement,
    NodeIdentifier,
    NodeInteger,
    NodeChar,
    NodeString,
    NodeArray,
    NodeStructInstance,
    NodeLambdaFunction,
    NodeSizeof,
    NodeFunctionCall,
    NodeMethodCall,
    NodeDereference,
    NodeIndex,
    NodeGetField,

    NodeAnd,
    NodeOr,
    NodeNot,
    NodeBitwiseAnd,
    NodeBitwiseOr,
    NodeBitwiseXor,
    NodeBitwiseNot,
    NodeBitwiseShiftLeft,
    NodeBitwiseShiftRight,
    NodeAddition,
    NodeSubtraction,
    NodeMultiplication,
    NodeDivision,
    NodeModulo,
    NodeLess,
    NodeGreater,
    NodeEqual,
    NodeLessEqual,
    NodeGreaterEqual,
    NodeNotEqual,
};

struct Node;
struct Module;
struct Block;
struct Include;
struct Test;
struct If;
struct While;
struct FunctionDefinition;
struct Prototype;
struct GlobalDefinition;
struct Definition;
struct TypeDefinition;
struct Return;
struct Break;
struct Continue;
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
struct LambdaFunction;
struct Sizeof;
struct Alloc;
struct Free;
struct FunctionCall;
struct MethodCall;
struct Dereference;
struct Index;
struct GetField;


struct FunctionSignature {
    struct Vector identifiers;
    struct Vector types;
    struct TypeNode *return_type;
    struct Vector addressed;
    bool propagate_allocator;
};

struct Node {
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
    void *node_ptr;
    enum NodeType node_type;
    struct TypeNode *type;
};

struct Module {
    struct Vector statement_list;
};

struct Block {
    struct Vector statement_list;
    const char *label;
};

struct Include {
    struct Vector statement_list;
};

struct Test {
    const char *name;
    struct Node *block;
    struct TypeNode *type;
};

struct If {
    struct Vector condition_list;
    struct Vector block_list;
    struct Node *else_block;
};

struct While {
    struct Node *condition;
    struct Node *block;
    struct Node *else_block;
    const char *label;
};

struct FunctionDefinition {
    struct TypeNode *caller_type;
    const char *name;
    struct FunctionSignature *signature;
    struct Node *block;
    bool external;
    bool global;
    struct TypeNode *type;
};

struct Prototype {
    struct TypeNode *caller_type;
    const char *name;
    struct FunctionSignature *signature;
    struct TypeNode *type;
};

struct GlobalDefinition {
    const char *identifier;
    struct TypeNode *type;
    struct Node *value;
};

struct Definition {
    const char *identifier;
    struct TypeNode *type;
    struct Node *value;
    bool addressed;
};

struct TypeDefinition {
    const char *identifier;
    struct TypeNode *type;
};

struct Return {
    struct Node *expression;
    const char *label;
};

struct Break {
    struct Node *expression;
    const char *label;
};

struct Continue {
    const char *label;
};

struct As {
    struct Node *expression;
    struct TypeNode *type;
};

struct Assignment {
    struct Node *dst, *src;
};

struct Movement {
    struct Node *dst, *src;
};

struct Identifier {
    const char *identifier;
    bool address;
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

struct LambdaFunction {
    struct FunctionSignature *signature;
    struct Node *block;
};

struct Sizeof {
    struct TypeNode *type;
};

struct FunctionCall {
    struct Node *function;
    struct Vector arguments;
    struct Node *propagate_allocator;
};

struct MethodCall {
    struct Node *caller;
    const char *function;
    struct Vector arguments;
    const char *name;
    struct Node *function_definition;
};

struct Dereference {
    struct Node *expression;
};

struct Index {
    struct Node *left, *right;
    bool address;
};

struct GetField {
    struct Node *left;
    const char *field;
    bool address;
    int phase;
};

struct BinaryOperator {
    struct Node *left, *right;
};

struct FunctionSignature *create_function_signature(struct Vector identifiers, struct Vector types, struct TypeNode *return_type);
struct Node *create_node(void *ptr, enum NodeType type);
struct Node *create_module(struct Vector statement_list);
struct Node *create_block(struct Vector statement_list, const char *label);
struct Node *create_include(struct Vector statement_list);
struct Node *create_test(const char *name, struct Node *block);
struct Node *create_if(struct Vector condition_list, struct Vector block_list, struct Node *else_block);
struct Node *create_while(struct Node *condition, struct Node *block, struct Node *else_block, const char *label);
struct Node *create_function_definition(struct TypeNode *caller_type, const char *name, struct FunctionSignature *signature, struct Node *block, bool external, bool global);
struct Node *create_prototype(struct TypeNode *caller_type, const char *name, struct FunctionSignature *signature);
struct Node *create_global_definition(const char *identifier, struct TypeNode *type, struct Node *value);
struct Node *create_definition(const char *identifier, struct TypeNode *type, struct Node *value);
struct Node *create_type_definition(const char *identifier, struct TypeNode *type);
struct Node *create_return(struct Node *expression, const char *label);
struct Node *create_break(struct Node *expression, const char *label);
struct Node *create_continue(const char *label);
struct Node *create_as(struct Node *expression, struct TypeNode *type);
struct Node *create_assignment(struct Node *dst, struct Node *src);
struct Node *create_movement(struct Node *dst, struct Node *src);
struct Node *create_identifier(const char *identifier, bool address);
struct Node *create_integer(long value);
struct Node *create_char(long value);
struct Node *create_string(const char *value);
struct Node *create_array(struct Vector values);
struct Node *create_struct_instance(struct Vector names, struct Vector values);
struct Node *create_lambda_function(struct FunctionSignature *signature, struct Node *block);
struct Node *create_sizeof(struct TypeNode *type);
struct Node *create_function_call(struct Node *function, struct Vector arguments, struct Node *propagate_allocator);
struct Node *create_method_call(struct Node *caller, const char *function, struct Vector arguments);
struct Node *create_dereference(struct Node *expression);
struct Node *create_index(struct Node *left, struct Node *right, bool address);
struct Node *create_get_field(struct Node *left, const char *field, bool address);
struct Node *create_binary_operator(enum NodeType node_type, struct Node *left, struct Node *right);
