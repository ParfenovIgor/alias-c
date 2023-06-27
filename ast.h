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

typedef int Type;
#define Type_Int 0
#define Type_Ptr 1

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
    void *node_ptr;
    int node_type;
    //virtual ~Node(){};
    //virtual void Validate(VLContext &context) = 0;
    //virtual void Compile(std::ostream &out, CPContext &context) = 0;
    //int line_begin, position_begin, line_end, position_end;
    //std::string filename;
};

struct Block {
    int line_begin, position_begin, line_end, position_end;
    //std::vector <std::shared_ptr <Statement>> statement_list;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Asm {
    const char *code;
};

struct If {
    //std::vector < std::pair <std::shared_ptr <Expression>, std::shared_ptr<Block>>> branch_list;
    //std::shared_ptr <Block> else_body;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct While {
    //std::shared_ptr <Expression> expression;
    //std::shared_ptr <Block> block;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
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
    //std::string identifier;
    //Type type;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Assignment {
    //std::string identifier;
    //std::shared_ptr <Expression> value;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Movement {
    //std::string identifier;
    //std::shared_ptr <Expression> value;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct MovementString {
    //std::string identifier;
    //std::string value;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Assumption {
    //std::string identifier;
    //std::shared_ptr <Expression> left, right;
    //std::shared_ptr <Statement> statement;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Identifier {
    //std::string identifier;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Integer {
    //int value;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Alloc {
    //std::shared_ptr <Expression> expression;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Free {
    //std::shared_ptr <Expression> arg;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct FunctionCall {
    //std::string identifier;
    //std::vector <std::pair <std::string, std::shared_ptr <Expression>>> metavariables;
    //std::vector <std::string> arguments;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Dereference {
    //std::shared_ptr <Expression> arg;
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Addition {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Subtraction {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Multiplication {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Division {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Less {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

struct Equal {
    //void Validate(VLContext &context);
    //void Compile(std::ostream &out, CPContext &context);
};

#endif // AST_H_INCLUDED
