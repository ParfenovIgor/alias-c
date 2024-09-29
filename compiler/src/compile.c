#include <ast.h>
#include <compile.h>
#include <settings.h>
#include <vector.h>
#include <exception.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

const char *regs[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};

int findInLocal(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->variable_local_name);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->variable_local_name[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findInArguments(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->variable_argument_name);
    for (int i = 0; i < sz; i++) {
        if (_strcmp(context->variable_argument_name[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findFunctionIndex(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->function_name);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->function_name[i], identifier) == 0) {
            return *context->function_name_index[i];
        }
    }
    print_string(STDOUT, "Error: function identifier not found\n");
    posix_exit(1);
}

struct FunctionSignature *findFunctionSignature(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->function_name);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->function_name[i], identifier) == 0) {
            return context->function_signature[i];
        }
    }
    print_string(STDOUT, "Error: function identifier not found\n");
    posix_exit(1);
}

void findIdentifier(const char *identifier, struct CPContext *context) {
    int cnt_args = get_size((void**)context->variable_argument_name);
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        print_stringi(context->outputFileDescriptor, "[rbp + ", -(idx + 1 + cnt_args) * 8, "]");
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            print_string(STDOUT, "Error: identifier not found\n");
            posix_exit(1);
        }
        print_stringi(context->outputFileDescriptor, "[rbp + ", -(idx + 1) * 8, "]");
    }
}

struct Type *findType(const char *identifier, struct CPContext *context) {
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        return context->variable_local_type[idx];
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            print_string(STDOUT, "Error: identifier not found\n");
            posix_exit(1);
        }
        return context->variable_argument_type[idx];
    }
}

struct Struct *findStruct(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->structs);
    for (int i = 0; i < sz; i++) {
        if (_strcmp(context->structs[i]->name, identifier) == 0) {
            return context->structs[i];
        }
    }
    return NULL;
}

struct Type *CompileNode(struct Node *node, struct CPContext *context);

void Compile(struct Node *node, struct Settings *settings) {
    print_string(settings->outputFileDescriptor, "; ");
    print_string(settings->outputFileDescriptor, node->filename);
    print_string(settings->outputFileDescriptor, " ");
    print_int   (settings->outputFileDescriptor, node->line_begin + 1);
    print_string(settings->outputFileDescriptor, ":");
    print_int   (settings->outputFileDescriptor, node->position_begin + 1);
    print_string(settings->outputFileDescriptor, " -> program\n");

    print_string(settings->outputFileDescriptor, "section .text\n");

    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variable_local_name = (const char**)_malloc(sizeof(const char*));
    context->variable_local_name[0] = NULL;
    context->variable_local_type = (struct Type**)_malloc(sizeof(struct Type*));
    context->variable_local_type[0] = NULL;
    context->variable_argument_name = (const char**)_malloc(sizeof(const char*));
    context->variable_argument_name[0] = NULL;
    context->variable_argument_type = (struct Type**)_malloc(sizeof(struct Type*));
    context->variable_argument_type[0] = NULL;
    context->function_name = (const char**)_malloc(sizeof(const char*));
    context->function_name[0] = NULL;
    context->function_name_index = (int**)_malloc(sizeof(int*));
    context->function_name_index[0] = NULL;
    context->function_signature = (struct FunctionSignature**)_malloc(sizeof(struct FunctionSignature*));
    context->function_signature[0] = NULL;
    context->structs = (struct Struct**)_malloc(sizeof(struct Struct*));
    context->structs[0] = NULL;
    context->function_index = 0;
    context->branch_index = 0;
    context->outputFileDescriptor = settings->outputFileDescriptor;

    CompileNode(node, context);
    print_string(settings->outputFileDescriptor, "leave\n");
    print_string(settings->outputFileDescriptor, "ret\n");
}

void CompileBlock(struct Node *node, struct Block *this, struct CPContext *context) {
    struct Node **statement = this->statement_list;
    int old_cnt_local_var = get_size((void**)context->variable_local_name);
    int old_cnt_functions = get_size((void**)context->function_name);
    while (*statement != NULL) {
        CompileNode(*statement, context);
        statement++;
    }
    int cnt_local_var = get_size((void**)context->variable_local_name);
    int cnt_functions = get_size((void**)context->function_name);
    print_stringi(context->outputFileDescriptor, "add rsp, ", 8 * (cnt_local_var - old_cnt_local_var), "\n");
    for (int i = 0; i < cnt_local_var - old_cnt_local_var; i++) {
        context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
        context->variable_local_type = (struct Type**)pop_back((void**)context->variable_local_type);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        context->function_name = (const char**)pop_back((void**)context->function_name);
        context->function_name_index = (int**)pop_back((void**)context->function_name_index);
        context->function_signature = (struct FunctionSignature**)pop_back((void**)context->function_signature);
    }
}

void CompileAsm(struct Node *node, struct Asm *this, struct CPContext *context) {
    print_string(context->outputFileDescriptor, this->code);
    print_string(context->outputFileDescriptor, "\n");
}

void CompileIf(struct Node *node, struct If *this, struct CPContext *context) {
    CompileNode(this->condition_list[0], context);
    int idx = context->branch_index;
    context->branch_index++;
    print_string(context->outputFileDescriptor, "cmp qword [rsp - 8], 0\n");
    print_stringi(context->outputFileDescriptor, "je _if_else", idx, "\n");
    CompileNode(this->block_list[0], context);
    print_stringi(context->outputFileDescriptor, "jmp _if_end", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_if_else", idx, ":\n");
    if (this->else_block) {
        CompileNode(this->else_block, context);
    }
    print_stringi(context->outputFileDescriptor, "_if_end", idx, ":\n");
}

void CompileWhile(struct Node *node, struct While *this, struct CPContext *context) {
    int idx = context->branch_index;
    context->branch_index++;
    print_stringi(context->outputFileDescriptor, "_while", idx, ":\n");
    CompileNode(this->condition, context);
    print_string(context->outputFileDescriptor, "cmp qword [rsp - 8], 0\n");
    print_stringi(context->outputFileDescriptor, "je _while_end", idx, "\n");
    CompileNode(this->block, context);
    print_stringi(context->outputFileDescriptor, "jmp _while", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_while_end", idx, ":\n");
}

void CompileFunctionDefinition(struct Node *node, struct FunctionDefinition *this, struct CPContext *context) {
    char *identifier, *identifier_end;
    int index;
    if (this->external) {
        identifier = _strdup(this->name);
        identifier_end = concat("_funend", this->name);
        index = -1;
    }
    else {
        identifier = concat("_fun", to_string(context->function_index));
        identifier_end = concat("_funend", to_string(context->function_index));
        index = context->function_index;
        context->function_index++;
    }

    if (this->external) {
        print_string3(context->outputFileDescriptor, "global ", identifier, "\n");
    }
    print_string3(context->outputFileDescriptor, "jmp ", identifier_end, "\n");
    print_string2(context->outputFileDescriptor, identifier, ":\n");
    print_string(context->outputFileDescriptor, "push rbp\n");
    print_string(context->outputFileDescriptor, "mov rbp, rsp\n");

    const char **variable_local_name_tmp = context->variable_local_name;
    struct Type **variable_local_type_tmp = context->variable_local_type;
    const char **variable_argument_name_tmp = context->variable_argument_name;
    struct Type **variable_argument_type_tmp = context->variable_argument_type;
    context->variable_local_name = (const char**)_malloc(sizeof(const char*));
    context->variable_local_name[0] = NULL;
    context->variable_local_type = (struct Type**)_malloc(sizeof(struct Type*));
    context->variable_local_type[0] = NULL;
    context->variable_argument_name = (const char**)_malloc(sizeof(const char*));
    context->variable_argument_name[0] = NULL;
    context->variable_argument_type = (struct Type**)_malloc(sizeof(struct Type*));
    context->variable_argument_type[0] = NULL;

    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = index;
    context->function_name = (const char**)push_back((void**)context->function_name, _strdup(this->name));
    context->function_name_index = (int**)push_back((void**)context->function_name_index, index_ptr);
    context->function_signature = (struct FunctionSignature**)push_back((void**)context->function_signature, this->signature);

    int sz = get_size((void**)this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        print_string3(context->outputFileDescriptor, "push ", regs[i], "\n");
        context->variable_argument_name = (const char**)push_back((void**)context->variable_argument_name, (void*)this->signature->identifiers[i]);
        context->variable_argument_type = (struct Type**)push_back((void**)context->variable_argument_type, CopyType(this->signature->types[i]));
    }
    CompileNode(this->block, context);
    
    print_stringi(context->outputFileDescriptor, "add rsp, ", sz * 8, "\n");
    for (int i = 0; i < sz; i++) {
        _free((void*)context->variable_argument_name[i]);
        _free((void*)context->variable_argument_type[i]);
    }
    _free((void*)context->variable_local_name);
    _free((void*)context->variable_local_type);
    _free((void*)context->variable_argument_name);
    _free((void*)context->variable_argument_type);

    context->variable_local_name = variable_local_name_tmp;
    context->variable_local_type = variable_local_type_tmp;
    context->variable_argument_name = variable_argument_name_tmp;
    context->variable_argument_type = variable_argument_type_tmp;

    print_string(context->outputFileDescriptor, "leave\n");
    print_string(context->outputFileDescriptor, "ret\n");
    print_string2(context->outputFileDescriptor, identifier_end, ":\n");
}

void CompilePrototype(struct Node *node, struct Prototype *this, struct CPContext *context) {
    print_string3(context->outputFileDescriptor, "extern ", this->name, "\n");
    context->function_name = (const char**)push_back((void**)context->function_name, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = -1;
    context->function_name_index = (int**)push_back((void**)context->function_name_index, index_ptr);
    context->function_signature = (struct FunctionSignature**)push_back((void**)context->function_signature, this->signature);
}

void CompileStructDefinition(struct Node *node, struct StructDefinition *this, struct CPContext *context) {
    struct Struct *_struct = (struct Struct*)_malloc(sizeof(struct Struct));
    _struct->name = this->name;
    _struct->identifiers = (const char**)_malloc(sizeof(const char*));
    _struct->identifiers[0] = NULL;
    _struct->types = (struct Type**)_malloc(sizeof(struct Type*));
    _struct->types[0] = NULL;
    for (int i = 0; i < get_size((void**)this->identifiers); i++) {
        _struct->identifiers = (const char**)push_back((void**)_struct->identifiers, (void*)this->identifiers[i]);
        _struct->types = (struct Type**)push_back((void**)_struct->types, this->types[i]);
    }
    context->structs = (struct Struct**)push_back((void**)context->structs, (void*)_struct);
}

void CompileDefinition(struct Node *node, struct Definition *this, struct CPContext *context) {
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, _strdup(this->identifier));
    struct Type *type = (struct Type*)_malloc(sizeof(int));
    type->identifier = _strdup(this->type->identifier);
    type->degree = this->type->degree;
    if (_strcmp(type->identifier, "int") && type->degree == 0) SemanticError("Only pointers to structures are supported", node);
    context->variable_local_type = (struct Type**)push_back((void**)context->variable_local_type, type);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
}

void CompileReturn(struct Node *node, struct Return *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
}

struct Type *CompileAs(struct Node *node, struct As *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->expression, context);
    _free(_type);
    return CopyType(this->type);
}

void CompileAssignment(struct Node *node, struct Assignment *this, struct CPContext *context) {
    if (this->dst->node_type != NodeIdentifier) {
        SemanticError("Identifier expected in assignment", node);
    }
    struct Identifier *_identifier = (struct Identifier*)(this->dst->node_ptr);
    struct Type *_type1 = findType(_identifier->identifier, context);
    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree) SemanticError("Assignment of not equal types", node);
    _free(_type2);

    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov ");
    findIdentifier(_identifier->identifier, context);
    print_string(context->outputFileDescriptor, ", rax\n");
}

void CompileMovement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->dst, context);
    if (_type1->degree == 0) SemanticError("Pointer expected in movement", node);

    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);

    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree + 1) {
        SemanticError("Movement of inappropriate types", node);
    }
    _free(_type1);
    _free(_type2);

    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rbx, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rax], rbx\n");
}

void CompileMovementString(struct Node *node, struct MovementString *this, struct CPContext *context) {
    /* int idx = context->branch_index;
    context->branch_index++;
    print_stringi(context->outputFileDescriptor, "jmp _strbufend", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_strbuf ", idx, " db ");
    int sz = _strlen(this->value);
    for (int i = 0; i < sz; i++) {
        print_int(context->outputFileDescriptor, (int)this->value[i]);
        if (i + 1 != sz) {
            print_string(context->outputFileDescriptor, ",");
        }
        else{
            print_string(context->outputFileDescriptor, "\n");
        }
    }
    print_stringi(context->outputFileDescriptor, "_strbufend", idx, ":\n");
    print_stringi(context->outputFileDescriptor, "mov rsi, _strbuf", idx, "\n");
    int phase = findPhase(this->identifier, context);
    print_stringi(context->outputFileDescriptor, "mov rdi, [rbp +", phase, "]\n");
    print_stringi(context->outputFileDescriptor, "mov rcx, ", sz, "\n");
    print_string(context->outputFileDescriptor, "rep movsb\n"); */
}

struct Type *CompileIdentifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    print_string(context->outputFileDescriptor, "mov rax, ");
    findIdentifier(this->identifier, context);
    print_string(context->outputFileDescriptor, "\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");

    struct Type *type = findType(this->identifier, context);
    return BuildType(type->identifier, type->degree);
}

struct Type *CompileInteger(struct Node *node, struct Integer *this, struct CPContext *context) {
    print_stringi(context->outputFileDescriptor, "mov qword [rsp - 8], ", this->value, "\n");
    return BuildType("int", 0);
}

struct Type *CompileSizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    int size;
    if (_strcmp(this->type->identifier, "int") == 0 || this->type->degree != 0) {
        size = 1;
    }
    else {
        struct Struct *_struct = findStruct(this->type->identifier, context);
        size = get_size((void**)_struct->identifiers);
    }
    print_stringi(context->outputFileDescriptor, "mov qword [rsp - 8], ", size * 8, "\n");
    return BuildType("int", 0);
}

struct Type *CompileFunctionCall(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    struct FunctionSignature *signature = findFunctionSignature(this->identifier, context);
    int sz = get_size((void**)this->arguments);
    if (sz != get_size((void**)signature->identifiers)) SemanticError("Incorrect number of arguments in function call", node);
    for (int i = 0; i < sz; i++) {
        struct Type *_type = CompileNode(this->arguments[i], context);
        if (_strcmp(_type->identifier, signature->types[i]->identifier) || _type->degree != signature->types[i]->degree) {
            SemanticError("Passing to function value of incorrect type", node);
        }
        _free(_type);
        print_string(context->outputFileDescriptor, "mov ");
        print_string(context->outputFileDescriptor, regs[i]);
        print_string(context->outputFileDescriptor, ", [rsp - 8]\n");
    }
    int idx = findFunctionIndex(this->identifier, context);
    if (idx == -1) {
        print_string3(context->outputFileDescriptor, "call ", this->identifier, "\n");
    }
    else {
        char *str = to_string(idx);
        print_string3(context->outputFileDescriptor, "call _fun", str, "\n");
        _free(str);
    }
    
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
    
    return CopyType(signature->return_type);
}

struct Type *CompileDereference(struct Node *node, struct Dereference *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->expression, context);
    if (_type->degree == 0) SemanticError("Dereference of not pointer value", node);
    _type->degree--;

    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rbx, [rax]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rbx\n");

    return _type;
}

struct Type *CompileGetField(struct Node *node, struct GetField *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree != 1) SemanticError("Pointer to structure expected", node);
    struct Struct *_struct = findStruct(_type->identifier, context);
    if (!_struct) SemanticError("Structure not found", node);
    _free(_type);

    int index = -1;
    int sz = get_size((void**)_struct->identifiers);
    for (int i = 0; i < sz; i++) {
        if (_strcmp(this->field, _struct->identifiers[i]) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) SemanticError("Structure doesn't have corresponding field", node);
    
    _type = CopyType(_struct->types[index]);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    if (!this->address) {    
        print_stringi(context->outputFileDescriptor, "mov rbx, [rax + ", index * 8, "]\n");
        print_string(context->outputFileDescriptor, "mov [rsp - 8], rbx\n");
    }
    else {
        print_stringi(context->outputFileDescriptor, "lea rbx, [rax + ", index * 8, "]\n");
        print_string(context->outputFileDescriptor, "mov [rsp - 8], rbx\n");
        _type->degree++;
    }

    return _type;
}

struct Type *CompileIndex(struct Node *node, struct Index *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree != 1) SemanticError("Pointer expected", node);

    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);

    struct Type *_type2 = CompileNode(this->right, context);
    if (_strcmp(_type2->identifier, "int") || _type2->degree != 0) SemanticError("Integer expected", node);
    _free(_type2);
    
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov rbx, 8\n");
    print_string(context->outputFileDescriptor, "mul rbx\n");
    print_string(context->outputFileDescriptor, "add rax, [rsp - 8]\n");
    if (!this->address) {
        print_string(context->outputFileDescriptor, "mov rbx, [rax]\n");
        print_string(context->outputFileDescriptor, "mov [rsp - 8], rbx\n");
        _type->degree--;
    }
    else {
        print_string(context->outputFileDescriptor, "lea rbx, [rax]\n");
        print_string(context->outputFileDescriptor, "mov [rsp - 8], rbx\n");
    }
    return _type;
}

struct Type *CompileAddition(struct Node *node, struct Addition *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in addition", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);

    _type = CompileNode(this->right, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in addition", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "add rax, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");

    return BuildType("int", 0);
}

struct Type *CompileSubtraction(struct Node *node, struct Subtraction *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "sub rax, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileMultiplication(struct Node *node, struct Multiplication *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in multiplication", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);
    
    _type = CompileNode(this->right, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in multiplication", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rdx, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mul rdx\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");

    return BuildType("int", 0);
}

struct Type *CompileDivision(struct Node *node, struct Division *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rdx, 0\n");
    print_string(context->outputFileDescriptor, "div qword [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileLess(struct Node *node, struct Less *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index++;
    print_stringi(context->outputFileDescriptor, "jl _set1_", idx, "\n");
    print_string(context->outputFileDescriptor, "mov qword [rsp - 8], 0\n");
    print_stringi(context->outputFileDescriptor, "jmp _setend", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_set1_", idx, ":\n");
    print_string(context->outputFileDescriptor, "mov qword [rsp - 8], 1\n");
    print_stringi(context->outputFileDescriptor, "_setend", idx, ":\n");
}

struct Type *CompileEqual(struct Node *node, struct Equal *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_local_name = (const char**)push_back((void**)context->variable_local_name, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_local_name = (const char**)pop_back((void**)context->variable_local_name);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index++;
    print_stringi(context->outputFileDescriptor, "jz _set1_", idx, "\n");
    print_string(context->outputFileDescriptor, "mov qword [rsp - 8], 0\n");
    print_stringi(context->outputFileDescriptor, "jmp _setend", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_set1_", idx, ":\n");
    print_string(context->outputFileDescriptor, "mov qword [rsp - 8], 1\n");
    print_stringi(context->outputFileDescriptor, "_setend", idx, ":\n");
}

struct Type *CompileNode(struct Node *node, struct CPContext *context) {
    print_string(context->outputFileDescriptor, "; ");
    print_string(context->outputFileDescriptor, node->filename);
    print_string(context->outputFileDescriptor, " ");
    print_int   (context->outputFileDescriptor, node->line_begin + 1);
    print_string(context->outputFileDescriptor, ":");
    print_int   (context->outputFileDescriptor, node->position_begin + 1);
    print_string(context->outputFileDescriptor, " -> ");

    if (node->node_type == NodeBlock) {
        print_string(context->outputFileDescriptor, "block\n");
        CompileBlock(node, (struct Block*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeAsm) {
        print_string(context->outputFileDescriptor, "asm\n");
        CompileAsm(node, (struct Asm*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeIf) {
        print_string(context->outputFileDescriptor, "if\n");
        CompileIf(node, (struct If*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeWhile) {
        print_string(context->outputFileDescriptor, "while\n");
        CompileWhile(node, (struct While*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        print_string(context->outputFileDescriptor, "function definition\n");
        CompileFunctionDefinition(node, (struct FunctionDefinition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodePrototype) {
        print_string(context->outputFileDescriptor, "prototype\n");
        CompilePrototype(node, (struct Prototype*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeStructDefinition) {
        print_string(context->outputFileDescriptor, "struct definition\n");
        CompileStructDefinition(node, (struct StructDefinition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeDefinition) {
        print_string(context->outputFileDescriptor, "definition\n");
        CompileDefinition(node, (struct Definition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeReturn) {
        print_string(context->outputFileDescriptor, "return\n");
        CompileReturn(node, (struct Return*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeAs) {
        print_string(context->outputFileDescriptor, "as\n");
        return CompileAs(node, (struct As*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAssignment) {
        print_string(context->outputFileDescriptor, "assignment\n");
        CompileAssignment(node, (struct Assignment*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeMovement) {
        print_string(context->outputFileDescriptor, "movement\n");
        CompileMovement(node, (struct Movement*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeMovementString) {
        print_string(context->outputFileDescriptor, "movement string\n");
        CompileMovementString(node, (struct MovementString*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeIdentifier) {
        print_string(context->outputFileDescriptor, "identifier\n");
        return CompileIdentifier(node, (struct Identifier*)node->node_ptr, context);
    }
    else if (node->node_type == NodeInteger) {
        print_string(context->outputFileDescriptor, "integer\n");
        return CompileInteger(node, (struct Integer*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSizeof) {
        print_string(context->outputFileDescriptor, "sizeof\n");
        return CompileSizeof(node, (struct Sizeof*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        print_string(context->outputFileDescriptor, "function call\n");
        return CompileFunctionCall(node, (struct FunctionCall*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDereference) {
        print_string(context->outputFileDescriptor, "dereference\n");
        return CompileDereference(node, (struct Dereference*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIndex) {
        print_string(context->outputFileDescriptor, "index\n");
        return CompileIndex(node, (struct Index*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGetField) {
        print_string(context->outputFileDescriptor, "get field\n");
        return CompileGetField(node, (struct GetField*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAddition) {
        print_string(context->outputFileDescriptor, "addition\n");
        return CompileAddition(node, (struct Addition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSubtraction) {
        print_string(context->outputFileDescriptor, "subtraction\n");
        return CompileSubtraction(node, (struct Subtraction*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMultiplication) {
        print_string(context->outputFileDescriptor, "multiplication\n");
        return CompileMultiplication(node, (struct Multiplication*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDivision) {
        print_string(context->outputFileDescriptor, "division\n");
        return CompileDivision(node, (struct Division*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLess) {
        print_string(context->outputFileDescriptor, "less\n");
        return CompileLess(node, (struct Less*)node->node_ptr, context);
    }
    else if (node->node_type == NodeEqual) {
        print_string(context->outputFileDescriptor, "equal\n");
        return CompileEqual(node, (struct Equal*)node->node_ptr, context);
    }
}
