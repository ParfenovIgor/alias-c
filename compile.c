#include <stdio.h>
#include "ast.h"
#include "compile.h"
#include "settings.h"
#include "vector.h"

const char **push_back_string(const char **a, const char *str);
const char **pop_back_string(const char **a);
int get_size_string(const char **a);

int findInLocal(const char *identifier, struct CPContext *context) {
    int sz = get_size_string(context->variable_stack);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->variable_stack[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findInArguments(const char *identifier, struct CPContext *context) {
    int sz = get_size_string(context->variable_arguments);
    for (int i = 0; i < sz; i++) {
        if (_strcmp(context->variable_arguments[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findFunctionIndex(const char *identifier, struct CPContext *context) {
    int sz = get_size_string(context->function_stack);
    for (int i = sz - 1; i >= 0; i--) {
        if (_strcmp(context->function_stack[i], identifier) == 0) {
            return *context->function_stack_index[i];
        }
    }
    print_string("Error: function identifier not found\n");
    program_exit(1);
}

int findPhase(const char *identifier, struct CPContext *context) {
    int idx = findInLocal(identifier, context);
    if (idx != -1) {
        return -(idx + 1) * 8;
    }
    else {
        idx = findInArguments(identifier, context);
        if (idx == -1) {
            print_string("Error: identifier not found\n");
            program_exit(1);
        }
        return (idx + 2) * 8;
    }
}

void CompileNode(struct Node *node, FILE *out, struct CPContext *context);

void Compile(struct Node *node, FILE *out, struct Settings *settings) {
    fprintf(out, "; %s %d:%d -> program\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    fprintf(out, "global _start\n");
    fprintf(out, "global main\n");
    fprintf(out, "section .text\n");
    fprintf(out, "_start:\n");
    fprintf(out, "call main\n");
    fprintf(out, "mov rax, 0x3c\n");
    fprintf(out, "mov rdi, 0\n");
    fprintf(out, "syscall\n");
    if (settings->topMain) {
        fprintf(out, "main:\n");
    }
    fprintf(out, "push rbp\n");
    fprintf(out, "mov rbp, rsp\n");

    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variable_stack = (const char**)_malloc(sizeof(const char*));
    context->variable_stack[0] = NULL;
    context->variable_stack_type = (enum Type**)_malloc(sizeof(enum Type*));
    context->variable_stack_type[0] = NULL;
    context->function_stack = (const char**)_malloc(sizeof(const char*));
    context->function_stack[0] = NULL;
    context->function_stack_index = (int**)_malloc(sizeof(int*));
    context->function_stack_index[0] = NULL;

    CompileNode(node, out, context);
    fprintf(out, "leave\n");
    fprintf(out, "ret\n");
}

void CompileBlock(struct Block *this, FILE *out, struct CPContext *context) {
    struct Node **statement = this->statement_list;
    int old_variable_stack_size = get_size_string(context->variable_stack);
    int old_function_stack_size = get_size_string(context->function_stack);
    while (*statement != NULL) {
        CompileNode(*statement, out, context);
        statement++;
    }
    int variable_stack_size = get_size_string(context->variable_stack);
    int function_stack_size = get_size_string(context->function_stack);
    fprintf(out, "add rsp, %d\n", 8 * (variable_stack_size - old_variable_stack_size));
    for (int i = 0; i < variable_stack_size - old_variable_stack_size; i++) {
        context->variable_stack = pop_back_string(context->variable_stack);
        context->variable_stack_type = pop_back_type(context->variable_stack_type);
    }
    for (int i = 0; i < function_stack_size - old_function_stack_size; i++) {
        context->function_stack = pop_back_string(context->function_stack);
    }
}

void CompileAsm(struct Asm *this, FILE *out, struct CPContext *context) {
    fprintf(out, "%s\n", this->code);
}

void CompileIf(struct If *this, FILE *out, struct CPContext *context) {
    CompileNode(this->condition_list[0], out, context);
    int idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "cmp [rsp - 8], dword 0\n");
    fprintf(out, "je _if_else%d\n", idx);
    CompileNode(this->block_list[0], out, context);
    fprintf(out, "jmp _if_end%d\n", idx);
    fprintf(out, "_if_else%d:\n", idx);
    if (this->else_block) {
        CompileNode(this->else_block, out, context);
    }
    fprintf(out, "_if_end%d:\n", idx);
}

void CompileWhile(struct While *this, FILE *out, struct CPContext *context) {
    int idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "_while%d:\n", idx);
    CompileNode(this->condition, out, context);
    fprintf(out, "cmp [rsp - 8], dword 0\n");
    fprintf(out, "je _while_end%d\n", idx);
    CompileNode(this->block, out, context);
    fprintf(out, "jmp _while%d\n", idx);
    fprintf(out, "_while_end%d:\n", idx);
}

void CompileFunctionDefinition(struct FunctionDefinition *this, FILE *out, struct CPContext *context) {
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
        fprintf(out, "global %s\n", identifier);
    }
    fprintf(out, "jmp %s\n", identifier_end);
    fprintf(out, "%s:\n", identifier);
    fprintf(out, "push rbp\n");
    fprintf(out, "mov rbp, rsp\n");
    int sz = get_size_string(this->signature->identifiers);
    fprintf(out, "sub rsp, %d\n", (sz + 2) * 8);

    const char **variable_stack_tmp = context->variable_stack;
    const char **variable_arguments_tmp = context->variable_arguments;
    context->variable_stack = (const char**)_malloc(sizeof(const char*));
    context->variable_stack[0] = NULL;
    context->variable_arguments = (const char**)_malloc(sizeof(const char*));
    context->variable_arguments[0] = NULL;
    context->function_stack = push_back_string(context->function_stack, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = index;
    context->function_stack_index = push_back_int(context->function_stack_index, index_ptr);

    sz = get_size_string(this->metavariables);
    for (int i = 0; i < sz; i++) {
        context->variable_arguments = push_back_string(context->variable_arguments, this->metavariables[i]);
    }
    sz = get_size_string(this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        context->variable_arguments = push_back_string(context->variable_arguments, this->signature->identifiers[i]);
    }
    CompileNode(this->block, out, context);
    
    sz = get_size_string(context->variable_stack);
    for (int i = 0; i < sz; i++) {
        _free((void*)context->variable_stack[i]);
    }
    sz = get_size_string(context->variable_arguments);
    for (int i = 0; i < sz; i++) {
        _free((void*)context->variable_arguments[i]);
    }

    context->variable_stack = variable_stack_tmp;
    context->variable_arguments = variable_arguments_tmp;

    fprintf(out, "leave\n");
    fprintf(out, "ret\n");
    fprintf(out, "%s:\n", identifier_end);
}

void CompilePrototype(struct Prototype *this, FILE *out, struct CPContext *context) {
    fprintf(out, "extern %s\n", this->name);
    context->function_stack = push_back_string(context->function_stack, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = -1;
    context->function_stack_index = push_back_int(context->function_stack_index, index_ptr);
}

void CompileDefinition(struct Definition *this, FILE *out, struct CPContext *context) {
    context->variable_stack = push_back_string(context->variable_stack, _strdup(this->identifier));
    enum Type *type = (enum Type*)_malloc(sizeof(int));
    *type = this->type;
    context->variable_stack_type = push_back_type(context->variable_stack_type, type);
    fprintf(out, "sub rsp, 8\n");
}

void CompileAssignment(struct Assignment *this, FILE *out, struct CPContext *context) {
    CompileNode(this->value, out, context);
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov [rbp + %d], rax\n", phase);
}

void CompileMovement(struct Movement *this, FILE *out, struct CPContext *context) {
    CompileNode(this->value, out, context);
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov rbx, [rbp + %d]\n", phase);
    fprintf(out, "mov [rbx], rax\n");
}

void CompileMovementString(struct MovementString *this, FILE *out, struct CPContext *context) {
    int idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "jmp _strbufend%d\n", idx);
    fprintf(out, "_strbuf%d db ", idx);
    int sz = _strlen(this->value);
    for (int i = 0; i < sz; i++) {
        fprintf(out, "%d", (int)this->value[i]);
        if (i + 1 != sz) {
            fprintf(out, ",");
        }
        else{
            fprintf(out, "\n");
        }
    }
    fprintf(out, "_strbufend%d:\n", idx);
    fprintf(out, "mov rsi, _strbuf%d\n", idx);
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rdi, [rbp + %d]\n", phase);
    fprintf(out, "mov rcx, %d\n", sz);
    fprintf(out, "rep movsb\n");
}

void CompileAssumption(struct Assumption *this, FILE *out, struct CPContext *context, const char *error) {
    int ind_error = context->branch_index;
    fprintf(out, "jmp aftererror%d\n", ind_error);
    fprintf(out, "error%d db \"%s\", 0xA\n", ind_error, error);
    fprintf(out, "aftererror%d:\n", ind_error);

    int phase = findPhase(this->identifier, context);
    int idx = context->branch_index;
    context->branch_index++;
    CompileNode(this->left, out, context);
    fprintf(out, "mov rax, [rbp + %d]\n", phase);
    fprintf(out, "sub rax, [rsp - 8]\n");
    fprintf(out, "jl _set1_%d\n", idx);
    fprintf(out, "jmp _setend%d\n", idx);
    fprintf(out, "_set1_%d:\n", idx);
    fprintf(out, "mov rax, 1\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "mov rsi, error%d\n", ind_error);
    fprintf(out, "mov rdx, %d\n", _strlen(error) + 1);
    fprintf(out, "syscall\n");
    fprintf(out, "mov rax, 0x3c\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "syscall\n");
    fprintf(out, "_setend%d:\n", idx);

    idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "sub rax, [rbp + %d]\n", phase);
    fprintf(out, "jl _set1_%d\n", idx);
    fprintf(out, "jmp _setend%d\n", idx);
    fprintf(out, "_set1_%d:\n", idx);
    fprintf(out, "mov rax, 1\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "mov rsi, error%d\n", ind_error);
    fprintf(out, "mov rdx, %d\n", _strlen(error) + 1);
    fprintf(out, "syscall\n");
    fprintf(out, "mov rax, 0x3c\n");
    fprintf(out, "mov rdi, 1\n");
    fprintf(out, "syscall\n");
    fprintf(out, "_setend%d:\n", idx);

    CompileNode(this->statement, out, context);
}

void CompileIdentifier(struct Identifier *this, FILE *out, struct CPContext *context) {
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rax, [rbp + %d]\n", phase);
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileInteger(struct Integer *this, FILE *out, struct CPContext *context) {
    fprintf(out, "mov [rsp - 8], dword %d\n", this->value);
}

void CompileAlloc(struct Alloc *this, FILE *out, struct CPContext *context) {
    CompileNode(this->expression, out, context);
    fprintf(out, "push dword [rsp - 8]\n");
    fprintf(out, "call malloc\n");
    fprintf(out, "add rsp, 8\n");
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileFree(struct Free *this, FILE *out, struct CPContext *context) {
    CompileNode(this->expression, out, context);
    fprintf(out, "push dword [rsp - 8]\n");
    fprintf(out, "call free\n");
    fprintf(out, "add rsp, 8\n");
}

void CompileFunctionCall(struct FunctionCall *this, FILE *out, struct CPContext *context) {
    int sz1 = get_size_string(this->arguments);
    for (int i = sz1 - 1; i >= 0; i--) {
        int phase = findPhase(this->arguments[i], context);
        fprintf(out, "push dword [rbp + %d]\n", phase);
    }
    int sz2 = get_size_string(this->metavariable_name);
    for (int i = sz2 - 1; i >= 0; i--) {
        CompileNode(this->metavariable_value[i], out, context);
        fprintf(out, "push dword [rsp - 8]\n");
    }
    int idx = findFunctionIndex(this->identifier, context);
    if (idx == -1) {
        fprintf(out, "call %s\n", this->identifier);
    }
    else {
        char *str = to_string(idx);
        fprintf(out, "call _fun%s\n", str);
        _free(str);
    }
    fprintf(out, "add rsp, %d\n", (sz1 + sz2) * 8);
    for (int i = sz1 - 1; i >= 0; i--) {
        int phase = findPhase(this->arguments[i], context);
        fprintf(out, "mov rax, [rsp - %d]\n", (sz1 - i) * 8);
        fprintf(out, "mov [rbp + %d], rax\n", phase);
    }
}

void CompileDereference(struct Dereference *this, FILE *out, struct CPContext *context) {
    CompileNode(this->expression, out, context);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov rbx, [rax]\n");
    fprintf(out, "mov [rsp - 8], rbx\n");
}

void CompileAddition(struct Addition *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "add rax, [rsp - 16]\n");
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileSubtraction(struct Subtraction *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "sub rax, [rsp - 16]\n");
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileMultiplication(struct Multiplication *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov rdx, [rsp - 16]\n");
    fprintf(out, "mul rdx\n");
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileDivision(struct Division *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov rdx, 0\n");
    fprintf(out, "div dword [rsp - 16]\n");
    fprintf(out, "mov [rsp - 8], rax\n");
}

void CompileLess(struct Less *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "jl _set1_%d\n", idx);
    fprintf(out, "mov [rsp - 8], dword 0\n");
    fprintf(out, "jmp _setend%d\n", idx);
    fprintf(out, "_set1_%d:\n", idx);
    fprintf(out, "mov [rsp - 8], dword 1\n");
    fprintf(out, "_setend%d:\n", idx);
}

void CompileEqual(struct Equal *this, FILE *out, struct CPContext *context) {
    CompileNode(this->left, out, context);
    fprintf(out, "sub rsp, 8\n");
    const char *identifier = "__junk";
    context->variable_stack = push_back_string(context->variable_stack, identifier);
    CompileNode(this->right, out, context);
    fprintf(out, "add rsp, 8\n");
    context->variable_stack = pop_back_string(context->variable_stack);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index++;
    fprintf(out, "jz _set1_%d\n", idx);
    fprintf(out, "mov [rsp - 8], dword 0\n");
    fprintf(out, "jmp _setend%d\n", idx);
    fprintf(out, "_set1_%d:\n", idx);
    fprintf(out, "mov [rsp - 8], dword 1\n");
    fprintf(out, "_setend%d:\n", idx);
}

void CompileNode(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> ", node->filename, node->line_begin + 1, node->position_begin + 1);
    if (node->node_type == NodeBlock) {
        fprintf(out, "block\n");
        CompileBlock((struct Block*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeAsm) {
        fprintf(out, "asm\n");
        CompileAsm((struct Asm*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeIf) {
        fprintf(out, "if\n");
        CompileIf((struct If*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeWhile) {
        fprintf(out, "while\n");
        CompileWhile((struct While*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        fprintf(out, "function definition\n");
        CompileFunctionDefinition((struct FunctionDefinition*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodePrototype) {
        fprintf(out, "prototype\n");
        CompilePrototype((struct Prototype*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeDefinition) {
        fprintf(out, "definition\n");
        CompileDefinition((struct Definition*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeAssignment) {
        fprintf(out, "assignment\n");
        CompileAssignment((struct Assignment*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeMovement) {
        fprintf(out, "movement\n");
        CompileMovement((struct Movement*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeMovementString) {
        fprintf(out, "movement string\n");
        CompileMovementString((struct MovementString*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeAssumption) {
        fprintf(out, "assumption\n");
        char *error = (char*)_malloc(1000);
        snprintf(error, 1000, "Assumption fault in file %s on line %d position %d", node->filename, node->line_begin + 1, node->position_begin + 1);
        CompileAssumption((struct Assumption*)node->node_ptr, out, context, error);
    }
    else if (node->node_type == NodeIdentifier) {
        fprintf(out, "identifier\n");
        CompileIdentifier((struct Identifier*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeInteger) {
        fprintf(out, "integer\n");
        CompileInteger((struct Integer*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeAlloc) {
        fprintf(out, "alloc\n");
        CompileAlloc((struct Alloc*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeFree) {
        fprintf(out, "free\n");
        CompileFree((struct Free*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        fprintf(out, "function call\n");
        CompileFunctionCall((struct FunctionCall*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeDereference) {
        fprintf(out, "dereference\n");
        CompileDereference((struct Dereference*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeAddition) {
        fprintf(out, "addition\n");
        CompileAddition((struct Addition*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeSubtraction) {
        fprintf(out, "subtraction\n");
        CompileSubtraction((struct Subtraction*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeMultiplication) {
        fprintf(out, "multiplication\n");
        CompileMultiplication((struct Multiplication*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeDivision) {
        fprintf(out, "division\n");
        CompileDivision((struct Division*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeLess) {
        fprintf(out, "less\n");
        CompileLess((struct Less*)node->node_ptr, out, context);
    }
    else if (node->node_type == NodeEqual) {
        fprintf(out, "equal\n");
        CompileEqual((struct Equal*)node->node_ptr, out, context);
    }
}
