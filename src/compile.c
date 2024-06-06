#include "../include/ast.h"
#include "../include/compile.h"
#include "../include/settings.h"
#include "../include/vector.h"
#include "../include/posix.h"
#include "../include/exception.h"
#include "../stdlib/include/stdlib.h"
#include "../stdlib/include/string.h"

int findInLocal(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->variable_stack);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->variable_stack[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findInArguments(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->variable_arguments);
    for (int i = 0; i < sz; i++) {
        if (_strcmp(context->variable_arguments[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findFunctionIndex(const char *identifier, struct CPContext *context) {
    int sz = get_size((void**)context->function_stack);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->function_stack[i], identifier) == 0) {
            return *context->function_stack_index[i];
        }
    }
    print_string(0, "Error: function identifier not found\n");
    posix_exit(1);
}

int findPhase(const char *identifier, struct CPContext *context) {
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        return -(idx + 1) * 8;
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            print_string(0, "Error: identifier not found\n");
            posix_exit(1);
        }
        return (idx + 2) * 8;
    }
}

struct Type *findType(const char *identifier, struct CPContext *context) {
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        return context->variable_stack_type[idx];
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            print_string(0, "Error: identifier not found\n");
            posix_exit(1);
        }
        return context->variable_arguments_type[idx];
    }
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

    print_string(settings->outputFileDescriptor, "global _start\n");
    print_string(settings->outputFileDescriptor, "global main\n");
    print_string(settings->outputFileDescriptor, "extern malloc\n");
    print_string(settings->outputFileDescriptor, "extern free\n");
    print_string(settings->outputFileDescriptor, "section .text\n");
    print_string(settings->outputFileDescriptor, "_start:\n");
    print_string(settings->outputFileDescriptor, "call main\n");
    print_string(settings->outputFileDescriptor, "mov rax, 0x3c\n");
    print_string(settings->outputFileDescriptor, "mov rdi, 0\n");
    print_string(settings->outputFileDescriptor, "syscall\n");
    if (settings->topMain) {
        print_string(settings->outputFileDescriptor, "main:\n");
    }
    print_string(settings->outputFileDescriptor, "push rbp\n");
    print_string(settings->outputFileDescriptor, "mov rbp, rsp\n");

    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variable_stack = (const char**)_malloc(sizeof(const char*));
    context->variable_stack[0] = NULL;
    context->variable_stack_type = (struct Type**)_malloc(sizeof(struct Type*));
    context->variable_stack_type[0] = NULL;
    context->function_stack = (const char**)_malloc(sizeof(const char*));
    context->function_stack[0] = NULL;
    context->function_stack_index = (int**)_malloc(sizeof(int*));
    context->function_stack_index[0] = NULL;
    context->function_index = 0;
    context->branch_index = 0;
    context->outputFileDescriptor = settings->outputFileDescriptor;

    CompileNode(node, context);
    print_string(settings->outputFileDescriptor, "leave\n");
    print_string(settings->outputFileDescriptor, "ret\n");
}

void CompileBlock(struct Node *node, struct Block *this, struct CPContext *context) {
    struct Node **statement = this->statement_list;
    int old_variable_stack_size = get_size((void**)context->variable_stack);
    int old_function_stack_size = get_size((void**)context->function_stack);
    while (*statement != NULL) {
        CompileNode(*statement, context);
        statement++;
    }
    int variable_stack_size = get_size((void**)context->variable_stack);
    int function_stack_size = get_size((void**)context->function_stack);
    print_stringi(context->outputFileDescriptor, "add rsp, ", 8 * (variable_stack_size - old_variable_stack_size), "\n");
    for (int i = 0; i < variable_stack_size - old_variable_stack_size; i++) {
        context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
        context->variable_stack_type = (struct Type**)pop_back((void**)context->variable_stack_type);
    }
    for (int i = 0; i < function_stack_size - old_function_stack_size; i++) {
        context->function_stack = (const char**)pop_back((void**)context->function_stack);
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
    print_stringi(context->outputFileDescriptor, "_while:", idx, ":\n");
    CompileNode(this->condition, context);
    print_string(context->outputFileDescriptor, "cmp qword [rsp - 8], 0\n");
    print_stringi(context->outputFileDescriptor, "jmp _while_end", idx, "\n");
    CompileNode(this->block, context);
    print_stringi(context->outputFileDescriptor, "jmp _while", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_while_end", idx, "\n");
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
    int sz = get_size((void**)this->signature->identifiers);
    print_stringi(context->outputFileDescriptor, "sub rsp, ", (sz + 2) * 8, "\n");
    const char **variable_stack_tmp = context->variable_stack;
    const char **variable_arguments_tmp = context->variable_arguments;
    context->variable_stack = (const char**)_malloc(sizeof(const char*));
    context->variable_stack[0] = NULL;
    context->variable_arguments = (const char**)_malloc(sizeof(const char*));
    context->variable_arguments[0] = NULL;
    context->function_stack = (const char**)push_back((void**)context->function_stack, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = index;
    context->function_stack_index = (int**)push_back((void**)context->function_stack_index, index_ptr);

    sz = get_size((void**)this->metavariables);
    for (int i = 0; i < sz; i++) {
        context->variable_arguments = (const char**)push_back((void**)context->variable_arguments, (void*)this->metavariables[i]);
    }
    sz = get_size((void**)this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        context->variable_arguments = (const char**)push_back((void**)context->variable_arguments, (void*)this->signature->identifiers[i]);
    }
    CompileNode(this->block, context);
    
    sz = get_size((void**)context->variable_stack);
    for (int i = 0; i < sz; i++) {
        _free((void*)context->variable_stack[i]);
    }
    sz = get_size((void**)context->variable_arguments);
    for (int i = 0; i < sz; i++) {
        _free((void*)context->variable_arguments[i]);
    }

    context->variable_stack = variable_stack_tmp;
    context->variable_arguments = variable_arguments_tmp;

    print_string(context->outputFileDescriptor, "leave\n");
    print_string(context->outputFileDescriptor, "ret\n");
    print_string2(context->outputFileDescriptor, identifier_end, ":\n");
}

void CompilePrototype(struct Node *node, struct Prototype *this, struct CPContext *context) {
    print_string3(context->outputFileDescriptor, "extern ", this->name, "\n");
    context->function_stack = (const char**)push_back((void**)context->function_stack, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = -1;
    context->function_stack_index = (int**)push_back((void**)context->function_stack_index, index_ptr);
}

void CompileStructDefinition(struct Node *node, struct StructDefinition *this, struct CPContext *context) {
    
}

void CompileDefinition(struct Node *node, struct Definition *this, struct CPContext *context) {
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, _strdup(this->identifier));
    struct Type *type = (struct Type*)_malloc(sizeof(int));
    type->identifier = _strdup(this->type->identifier);
    type->degree = this->type->degree;
    context->variable_stack_type = (struct Type**)push_back((void**)context->variable_stack_type, type);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
}

void CompileAssignment(struct Node *node, struct Assignment *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->value, context);
    struct Type *_type2 = findType(this->identifier, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree) SemanticError("Assignment of not equal types", node);
    _free(_type1);

    int phase = findPhase(this->identifier, context);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_stringi(context->outputFileDescriptor, "mov [rbp + ", phase, "], rax\n");
}

void CompileMovement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->value, context);
    struct Type *_type2 = findType(this->identifier, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree + 1) SemanticError("Movement of inappropriate types", node);
    _free(_type1);

    CompileNode(this->value, context);
    int phase = findPhase(this->identifier, context);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_stringi(context->outputFileDescriptor, "mov rbx, [rbp + ", phase, "]\n");
    print_string(context->outputFileDescriptor, "mov [rbx], rax\n");
}

void CompileMovementString(struct Node *node, struct MovementString *this, struct CPContext *context) {
    int idx = context->branch_index;
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
    print_string(context->outputFileDescriptor, "rep movsb\n");
}

void CompileAssumption(struct Node *node, struct Assumption *this, struct CPContext *context) {
    int ind_error = context->branch_index;
    print_stringi(context->outputFileDescriptor, "jmp aftererror", ind_error, "\n");
    print_stringi(context->outputFileDescriptor, "error", ind_error, " db \"");
    
    char *str1 = concat("Assumption fault in file ", node->filename);
    char *str2 = concat(str1, " on line ");
    char *str3 = concat(str2, to_string(node->line_begin + 1));
    char *str4 = concat(str3, " position ");
    char *error = concat(str4, to_string(node->position_begin + 1));
    _free(str1);
    _free(str2);
    _free(str3);
    _free(str4);

    print_string2(context->outputFileDescriptor, error, "\", 0xA\n");
    print_stringi(context->outputFileDescriptor, "aftererror", ind_error, ":\n");

    int phase = findPhase(this->identifier, context);
    int idx = context->branch_index;
    context->branch_index++;
    CompileNode(this->left, context);
    print_stringi(context->outputFileDescriptor, "mov rax, [rbp + ", phase, "]\n");
    print_string(context->outputFileDescriptor, "sub rax, [rsp - 8]\n");
    print_stringi(context->outputFileDescriptor, "jl _set1_", idx, "\n");
    print_stringi(context->outputFileDescriptor, "jmp _setend", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_set1_", idx, ":\n");
    print_string(context->outputFileDescriptor, "mov rax, 1\n");
    print_string(context->outputFileDescriptor, "mov rdi, 1\n");
    print_stringi(context->outputFileDescriptor, "mov rsi, error", ind_error, "\n");
    print_stringi(context->outputFileDescriptor, "mov rdx, ", _strlen(error) + 1, "\n");
    print_string(context->outputFileDescriptor, "syscall\n");
    print_string(context->outputFileDescriptor, "mov rax, 0x3c\n");
    print_string(context->outputFileDescriptor, "mov rdi, 1\n");
    print_string(context->outputFileDescriptor, "syscall\n");
    print_stringi(context->outputFileDescriptor, "_setend", idx, ":\n");

    idx = context->branch_index;
    context->branch_index++;
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_stringi(context->outputFileDescriptor, "sub rax, [rbp + ", phase, "]\n");
    print_stringi(context->outputFileDescriptor, "jl _set1_", idx, "\n");
    print_stringi(context->outputFileDescriptor, "jmp _setend", idx, "\n");
    print_stringi(context->outputFileDescriptor, "_set1_", idx, ":\n");
    print_string(context->outputFileDescriptor, "mov rax, 1\n");
    print_string(context->outputFileDescriptor, "mov rdi, 1\n");
    print_stringi(context->outputFileDescriptor, "mov rsi, error", ind_error, "\n");
    print_stringi(context->outputFileDescriptor, "mov rdx, ", _strlen(error) + 1, "\n");
    print_string(context->outputFileDescriptor, "syscall\n");
    print_string(context->outputFileDescriptor, "mov rax, 0x3c\n");
    print_string(context->outputFileDescriptor, "mov rdi, 1\n");
    print_string(context->outputFileDescriptor, "syscall\n");
    print_stringi(context->outputFileDescriptor, "_setend", idx, ":\n");

    CompileNode(this->statement, context);
    _free(error);
}

struct Type *CompileIdentifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    int phase = findPhase(this->identifier, context);
    print_stringi(context->outputFileDescriptor, "mov rax, [rbp + ", phase, "]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");

    struct Type *type = findType(this->identifier, context);
    return BuildType(type->identifier, type->degree);
}

struct Type *CompileInteger(struct Node *node, struct Integer *this, struct CPContext *context) {
    print_stringi(context->outputFileDescriptor, "mov qword [rsp - 8], ", this->value, "\n");
    return BuildType("int", 0);
}

struct Type *CompileAlloc(struct Node *node, struct Alloc *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    print_string(context->outputFileDescriptor, "push qword [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "call malloc\n");
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileFree(struct Node *node, struct Free *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    print_string(context->outputFileDescriptor, "push qword [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "call free\n");
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
}

void CompileFunctionCall(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    int sz1 = get_size((void**)this->arguments);
    for (int i = sz1 - 1; i >= 0; i--) {
        int phase = findPhase(this->arguments[i], context);
        print_stringi(context->outputFileDescriptor, "push qword [rbp + ", phase, "]\n");
    }
    int sz2 = get_size((void**)this->metavariable_name);
    for (int i = sz2 - 1; i >= 0; i--) {
        CompileNode(this->metavariable_value[i], context);
        print_string(context->outputFileDescriptor, "push qword [rsp - 8]\n");
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
    print_stringi(context->outputFileDescriptor, "add rsp, ", (sz1 + sz2) * 8, "\n");
    for (int i = sz1 - 1; i >= 0; i--) {
        int phase = findPhase(this->arguments[i], context);
        print_stringi(context->outputFileDescriptor, "mov rax, [rsp - ", (sz1 - i) * 8, "]\n");
        print_stringi(context->outputFileDescriptor, "mov [rbp + ", phase, "], rax\n");
    }
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

struct Type *CompileAddition(struct Node *node, struct Addition *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in addition", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);

    _type = CompileNode(this->right, context);
    if (_strcmp(_type->identifier, "int") || _type->degree != 0) SemanticError("Integer expected in addition", node);
    _free(_type);

    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "add rax, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");

    return BuildType("int", 0);
}

struct Type *CompileSubtraction(struct Node *node, struct Subtraction *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "sub rax, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileMultiplication(struct Node *node, struct Multiplication *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rdx, [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mul rdx\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileDivision(struct Node *node, struct Division *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
    print_string(context->outputFileDescriptor, "mov rax, [rsp - 8]\n");
    print_string(context->outputFileDescriptor, "mov rdx, 0\n");
    print_string(context->outputFileDescriptor, "div qword [rsp - 16]\n");
    print_string(context->outputFileDescriptor, "mov [rsp - 8], rax\n");
}

struct Type *CompileLess(struct Node *node, struct Less *this, struct CPContext *context) {
    CompileNode(this->left, context);
    print_string(context->outputFileDescriptor, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
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
    context->variable_stack = (const char**)push_back((void**)context->variable_stack, (void*)identifier);
    CompileNode(this->right, context);
    print_string(context->outputFileDescriptor, "add rsp, 8\n");
    context->variable_stack = (const char**)pop_back((void**)context->variable_stack);
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
    else if (node->node_type == NodeAssumption) {
        print_string(context->outputFileDescriptor, "assumption\n");
        CompileAssumption(node, (struct Assumption*)node->node_ptr, context);
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
    else if (node->node_type == NodeAlloc) {
        print_string(context->outputFileDescriptor, "alloc\n");
        return CompileAlloc(node, (struct Alloc*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFree) {
        print_string(context->outputFileDescriptor, "free\n");
        return CompileFree(node, (struct Free*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        print_string(context->outputFileDescriptor, "function call\n");
        CompileFunctionCall(node, (struct FunctionCall*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeDereference) {
        print_string(context->outputFileDescriptor, "dereference\n");
        return CompileDereference(node, (struct Dereference*)node->node_ptr, context);
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
