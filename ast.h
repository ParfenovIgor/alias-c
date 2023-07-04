#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

enum NodeType {
    NodeBlock,
    NodeAsm,
    NodeIf,
    NodeWhile,
    NodeFunctionDefinition,
    NodePrototype,
    NodeDefinition,
    NodeAssignment,
    NodeMovement,
    NodeMovementString,
    NodeAssumption,
    NodeIdentifier,
    NodeInteger,
    NodeAlloc,
    NodeFree,
    NodeFunctionCall,
    NodeDereference,
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
struct Definition;
struct Assignment;
struct Movement;
struct MovementString;
struct Assumption;
struct Identifier;
struct Integer;
struct Alloc;
struct Free;
struct FunctionCall;
struct Dereference;
struct Addition;
struct Subtraction;
struct Multiplication;
struct Division;
struct Less;
struct Equal;

enum Type {
    TypeInt,
    TypePtr,
};

struct FunctionSignature {
    //std::vector < std::string > identifiers;
    //std::vector <Type> types;
    //std::vector < std::shared_ptr <Expression> > size_in, size_out;
    //std::vector <bool> is_const;
};

struct FunctionSignatureEvaluated {
    //std::vector < std::string > identifiers;
    //std::vector <Type> types;
    //std::vector <int> size_in, size_out;
    //std::vector <bool> is_const;
};

struct State {
    //std::vector < std::pair <int, int> > heap;
};

struct VLContext {
    //std::vector < std::string > variable_stack;
    //std::vector <Type> variable_type_stack;
    //std::vector <bool> variable_is_const_stack;
    //std::vector < std::string > function_stack;
    //std::vector < std::shared_ptr <FunctionSignature> > function_signature_stack;
    //std::vector <FunctionDefinition*> function_pointer_stack;
    //std::vector < std::set <FunctionSignatureEvaluated> > function_signature_validated;
    //std::vector <int> packet_size;
    //std::set <State> states;
    //std::vector < std::pair <std::string, int> > metavariable_stack;
};

struct CPContext {
    //std::vector <std::string> variable_stack;
    //std::vector <Type> variable_stack_type;
    //std::vector <std::string> variable_arguments;
    //std::vector <Type> variable_arguments_type;
    //std::vector < std::pair < std::string, int> > function_stack;
    //int function_index = 0;
    //int branch_index = 0;
};

struct Node {
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
    void *node_ptr;
    enum NodeType node_type;
    //virtual ~Node(){};
    //virtual void Validate(VLContext &context) = 0;
    //virtual void Compile(std::ostream &out, CPContext &context) = 0;
    //int line_begin, position_begin, line_end, position_end;
    //std::string filename;
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
    //std::string name;
    //std::vector <std::string> metavariables;
    //std::shared_ptr <FunctionSignature> signature;
    //std::shared_ptr <Block> body;
    //bool external;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Prototype {
    //std::string name;
    //std::vector <std::string> metavariables;
    //std::shared_ptr <FunctionSignature> signature;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Definition {
    const char *identifier;
    enum Type type;
    //std::string identifier;
    //Type type;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Assignment {
    const char *identifier;
    struct Node *value;
};

struct Movement {
    const char *identifier;
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
    const char **arguments;
};

struct Dereference {
    struct Node *arg;
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

#endif // AST_H_INCLUDED
