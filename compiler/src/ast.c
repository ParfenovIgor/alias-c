#include <ast.h>
#include <stdlib.h>

#define Alloc(T, x) struct T *x = (struct T*)_malloc(sizeof(struct T));

struct FunctionSignature *create_function_signature(struct Vector identifiers, struct Vector types, struct TypeNode *return_type) {
    Alloc(FunctionSignature, this);
    this->identifiers = identifiers;
    this->types = types;
    this->return_type = return_type;
    this->propagate_allocator = false;
    return this;
}

struct Node *create_node(void *ptr, enum NodeType type) {
    Alloc(Node, node);
    node->node_ptr = ptr;
    node->node_type = type;
    return node;
}

struct Node *create_module(struct Vector statement_list) {
    Alloc(Module, this);
    this->statement_list = statement_list;
    return create_node(this, NodeModule);
}

struct Node *create_block(struct Vector statement_list, const char *label) {
    Alloc(Block, this);
    this->statement_list = statement_list;
    this->label = label;
    return create_node(this, NodeBlock);
}

struct Node *create_include(struct Vector statement_list) {
    Alloc(Include, this);
    this->statement_list = statement_list;
    return create_node(this, NodeInclude);
}

struct Node *create_test(const char *name, struct Node *block) {
    Alloc(Test, this);
    this->name = name;
    this->block = block;
    return create_node(this, NodeTest);
}

struct Node *create_if(struct Vector condition_list, struct Vector block_list, struct Node *else_block) {
    Alloc(If, this);
    this->condition_list = condition_list;
    this->block_list = block_list;
    this->else_block = else_block;
    return create_node(this, NodeIf);
}

struct Node *create_while(struct Node *condition, struct Node *block, struct Node *else_block, const char *label) {
    Alloc(While, this);
    this->condition = condition;
    this->block = block;
    this->else_block = else_block;
    this->label = label;
    return create_node(this, NodeWhile);
}

struct Node *create_function_definition(struct TypeNode *caller_type, const char *name, struct FunctionSignature *signature, struct Node *block, bool external, bool global) {
    Alloc(FunctionDefinition, this);
    this->caller_type = caller_type;
    this->name = name;
    this->signature = signature;
    this->block = block;
    this->external = external;
    this->global = global;
    return create_node(this, NodeFunctionDefinition);
}

struct Node *create_prototype(struct TypeNode *caller_type, const char *name, struct FunctionSignature *signature) {
    Alloc(Prototype, this);
    this->caller_type = caller_type;
    this->name = name;
    this->signature = signature;
    return create_node(this, NodePrototype);
}

struct Node *create_global_definition(const char *identifier, struct TypeNode *type, struct Node *value) {
    Alloc(GlobalDefinition, this);
    this->identifier = identifier;
    this->type = type;
    this->value = value;
    return create_node(this, NodeGlobalDefinition);
}

struct Node *create_definition(const char *identifier, struct TypeNode *type, struct Node *value) {
    Alloc(Definition, this);
    this->identifier = identifier;
    this->type = type;
    this->value = value;
    return create_node(this, NodeDefinition);
}

struct Node *create_type_definition(const char *identifier, struct TypeNode *type) {
    Alloc(TypeDefinition, this);
    this->identifier = identifier;
    this->type = type;
    return create_node(this, NodeTypeDefinition);
}

struct Node *create_return(struct Node *expression, const char *label) {
    Alloc(Return, this);
    this->expression = expression;
    this->label = label;
    return create_node(this, NodeReturn);
}

struct Node *create_break(struct Node *expression, const char *label) {
    Alloc(Break, this);
    this->expression = expression;
    this->label = label;
    return create_node(this, NodeBreak);
}

struct Node *create_continue(const char *label) {
    Alloc(Continue, this);
    this->label = label;
    return create_node(this, NodeContinue);
}

struct Node *create_as(struct Node *expression, struct TypeNode *type) {
    Alloc(As, this);
    this->expression = expression;
    this->type = type;
    return create_node(this, NodeAs);
}

struct Node *create_assignment(struct Node *dst, struct Node *src) {
    Alloc(Assignment, this);
    this->dst = dst;
    this->src = src;
    return create_node(this, NodeAssignment);
}

struct Node *create_movement(struct Node *dst, struct Node *src) {
    Alloc(Movement, this);
    this->dst = dst;
    this->src = src;
    return create_node(this, NodeMovement);
}

struct Node *create_identifier(const char *identifier, bool address) {
    Alloc(Identifier, this);
    this->identifier = identifier;
    this->address = address;
    return create_node(this, NodeIdentifier);
}

struct Node *create_integer(long value) {
    Alloc(Integer, this);
    this->value = value;
    return create_node(this, NodeInteger);
}

struct Node *create_char(long value) {
    Alloc(Char, this);
    this->value = value;
    return create_node(this, NodeChar);
}

struct Node *create_string(const char *value) {
    Alloc(String, this);
    this->value = value;
    return create_node(this, NodeString);
}

struct Node *create_array(struct Vector values) {
    Alloc(Array, this);
    this->values = values;
    return create_node(this, NodeArray);
}

struct Node *create_struct_instance(struct Vector names, struct Vector values) {
    Alloc(StructInstance, this);
    this->names = names;
    this->values = values;
    return create_node(this, NodeStructInstance);
}

struct Node *create_lambda_function(struct FunctionSignature *signature, struct Node *block) {
    Alloc(LambdaFunction, this);
    this->signature = signature;
    this->block = block;
    return create_node(this, NodeLambdaFunction);
}

struct Node *create_sizeof(struct TypeNode *type) {
    Alloc(Sizeof, this);
    this->type = type;
    return create_node(this, NodeSizeof);
}

struct Node *create_function_call(struct Node *function, struct Vector arguments, struct Node *propagate_allocator) {
    Alloc(FunctionCall, this);
    this->function = function;
    this->arguments = arguments;
    this->propagate_allocator = propagate_allocator;
    return create_node(this, NodeFunctionCall);
}

struct Node *create_method_call(struct Node *caller, const char *function, struct Vector arguments) {
    Alloc(MethodCall, this);
    this->caller = caller;
    this->function = function;
    this->arguments = arguments;
    return create_node(this, NodeMethodCall);
}

struct Node *create_dereference(struct Node *expression) {
    Alloc(Dereference, this);
    this->expression = expression;
    return create_node(this, NodeDereference);
}

struct Node *create_index(struct Node *left, struct Node *right, bool address) {
    Alloc(Index, this);
    this->left = left;
    this->right = right;
    this->address = address;
    return create_node(this, NodeIndex);
}

struct Node *create_get_field(struct Node *left, const char *field, bool address) {
    Alloc(GetField, this);
    this->left = left;
    this->field = field;
    this->address = address;
    return create_node(this, NodeGetField);
}

struct Node *create_binary_operator(enum NodeType node_type, struct Node *left, struct Node *right) {
    Alloc(BinaryOperator, this);
    this->left = left;
    this->right = right;
    return create_node(this, node_type);
}
