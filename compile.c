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
        if (strcmp(context->variable_stack[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

int findInArguments(const char *identifier, struct CPContext *context) {
    int sz = get_size_string(context->variable_arguments);
    for (int i = 0; i < sz; i++) {
        if (strcmp(context->variable_arguments[i], identifier) == 0) {
            return i;
        }
    }
    return -1;
}

/*Type getVariableType(std::string id, Node *node, CPContext &context) {
    for (int i = (int)context.variable_stack.size() - 1; i >= 0; i--) {
        if (context.variable_stack[i] == id) {
            return context.variable_stack_type[i];
        }
    }
    for (int i = 0; i < (int)context.variable_arguments.size(); i++) {
        if (context.variable_arguments[i] == id) {
            return context.variable_arguments_type[i];
        }
    }
    std::cout << "Error: identifier not found" << std::endl;
    exit(1);
}

int findFunctionIndex(std::string &identifier, CPContext &context) {
    for (int i = (int)context.function_stack.size() - 1; i >= 0; i--) {
        if (context.function_stack[i].first == identifier) {
            return context.function_stack[i].second;
        }
    }
    std::cout << "Error: function identifier not found" << std::endl;
    exit(1);
}*/

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

    CompileNode(node, out, context);
    fprintf(out, "leave\n");
    fprintf(out, "ret\n");
}

void CompileBlock(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> block\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Block *this = (struct Block*)node->node_ptr;
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

void CompileAsm(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> asm\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Asm *this = (struct Asm*)node->node_ptr;
    fprintf(out, "%s\n", this->code);
}

void CompileIf(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> if\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct If *this = (struct If*)node->node_ptr;
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

void CompileWhile(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> while\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct While *this = (struct While*)node->node_ptr;
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

void CompileFunctionDefinition(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> function definition\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct FunctionDefinition *this = (struct FunctionDefinition*)node->node_ptr;
    
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

void CompilePrototype(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> prototype\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Prototype *this = (struct Prototype*)node->node_ptr;
    fprintf(out, "extern %s\n", this->name);
    context->function_stack = push_back_string(context->function_stack, _strdup(this->name));
    int *index_ptr = (int*)_malloc(sizeof(int));
    *index_ptr = -1;
    context->function_stack_index = push_back_int(context->function_stack_index, index_ptr);
}

void CompileDefinition(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> definition\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Definition *this = (struct Definition*)node->node_ptr;
    context->variable_stack = push_back_string(context->variable_stack, _strdup(this->identifier));
    enum Type *type = (enum Type*)_malloc(sizeof(int));
    *type = this->type;
    context->variable_stack_type = push_back_type(context->variable_stack_type, type);
    fprintf(out, "sub rsp, 8\n");
}

void CompileAssignment(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> assignment\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Assignment *this = (struct Assignment*)node->node_ptr;
    CompileNode(this->value, out, context);
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov [rbp + %d], rax\n", phase);
}

void CompileMovement(struct Node *node, FILE *out, struct CPContext *context) {
    fprintf(out, "; %s %d:%d -> movement\n", node->filename, node->line_begin + 1, node->position_begin + 1);
    struct Movement *this = (struct Movement*)node->node_ptr;
    CompileNode(this->value, out, context);
    int phase = findPhase(this->identifier, context);
    fprintf(out, "mov rax, [rsp - 8]\n");
    fprintf(out, "mov rbx, [rbp + %d]\n", phase);
    fprintf(out, "mov [rbx], rax\n");
}

/*void Movement::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> movement\n";
    value->Compile(out, context);
    int phase = findPhase(identifier, context);
    out << "mov eax, [esp - 4]\n";
    out << "mov ebx, [ebp + " << phase << "]\n";
    out << "mov [ebx], eax\n";
}

void MovementString::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> movement string\n";
    int idx = context.branch_index;
    context.branch_index++;
    out << "jmp _strbufend" << idx << "\n";
    out << "_strbuf" << idx << " db ";
    for (int i = 0; i < (int)value.size(); i++) {
        out << (int)value[i];
        if (i + 1 != (int)value.size()) {
            out << ",";
        }
        else {
            out << "\n";
        }
    }
    out << "_strbufend" << idx << ":\n";
    out << "mov esi, _strbuf" << idx << "\n";
    int phase = findPhase(identifier, context);
    out << "mov edi, [ebp + " << phase << "]\n";
    out << "mov ecx, " << (int)value.size() << "\n";
    out << "rep movsb\n";
}

void Assumption::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> assumption\n";

    int ind_error = context.branch_index++;
    out << "jmp aftererror" << ind_error << "\n";
    std::string error = "Assumption fault in file " + filename + " on line " + std::to_string(line_begin + 1) + " position " + std::to_string(position_begin + 1);
    out << "error" << ind_error << " db \"" << error << "\", 0xA\n";
    out << "aftererror" << ind_error << ":\n";

    int phase = findPhase(identifier, context);
    int idx = context.branch_index++;
    left->Compile(out, context);
    out << "mov eax, [ebp + " << phase << "]\n";
    out << "sub eax, [esp - 4]\n";
    out << "jl " << "_set1_" << idx << "\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov eax, 4\n";
    out << "mov ebx, 1\n";
    out << "mov ecx, error" << ind_error << "\n";
    out << "mov edx, " << error.size() + 1 << "\n";
    out << "int 0x80\n";
    out << "mov eax, 1\n";
    out << "mov ebx, 1\n";
    out << "int 0x80\n";
    out << "_setend" << idx << ":\n";

    idx = context.branch_index++;
    right->Compile(out, context);
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [ebp + " << phase << "]\n";
    out << "jl " << "_set1_" << idx << "\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov eax, 4\n";
    out << "mov ebx, 1\n";
    out << "mov ecx, error" << ind_error << "\n";
    out << "mov edx, " << error.size() + 1 << "\n";
    out << "int 0x80\n";
    out << "mov eax, 1\n";
    out << "mov ebx, 1\n";
    out << "int 0x80\n";
    out << "_setend" << idx << ":\n";

    statement->Compile(out, context);
}

void Identifier::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> identifier\n";
    int phase = findPhase(identifier, context);
    out << "mov eax, [ebp + " << phase << "]\n";
    out << "mov [esp - 4], eax\n";
}

void Integer::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> integer\n";
    out << "mov [esp - 4], dword " << value << "\n";
}

void Alloc::Compile(std::ostream &out, CPContext &context) {
    int line_begin = expression->line_begin;
    int position_begin = expression->position_begin;
    int line_end = expression->line_end;
    int position_end = expression->position_end;
    std::string filename = expression->filename;

    auto _multiplication = std::make_shared <AST::Multiplication> ();
    _multiplication->line_begin = line_begin;
    _multiplication->position_begin = position_begin;
    _multiplication->line_end = line_end;
    _multiplication->position_end = position_end;
    _multiplication->filename = filename;

    auto _integer  = std::make_shared <AST::Integer> ();
    _integer->line_begin = line_begin;
    _integer->position_begin = position_begin;
    _integer->line_end = line_end;
    _integer->position_end = position_end;
    _integer->filename = filename;

    _integer->value = 4;
    _multiplication->left = expression;
    _multiplication->right = _integer;
    expression = _multiplication;

    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> alloc\n";
    expression->Compile(out, context);
    out << "push dword [esp - 4]\n";
    out << "call malloc\n";
    out << "add esp, 4\n";
    out << "mov [esp - 4], eax\n";
}

void Free::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> free\n";
    arg->Compile(out, context);
    out << "push dword [esp - 4]\n";
    out << "call free\n";
    out << "add esp, 4\n";
}

void FunctionCall::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> function call\n";
    for (int i = (int)arguments.size() - 1; i >= 0; i--) {
        int phase = findPhase(arguments[i], context);
        out << "push dword [ebp + " << phase << "]\n";
    }
    for (int i = (int)metavariables.size() - 1; i >= 0; i--) {
        metavariables[i].second->Compile(out, context);
        out << "push dword [esp - 4]\n";
    }
    int idx = findFunctionIndex(identifier, context);
    if (idx == -1) {
        out << "call " << identifier << "\n";
    }
    else {
        out << "call _fun" << idx << "\n";
    }
    out << "add esp, " << (int)(arguments.size() + metavariables.size()) * 4 << "\n";
    for (int i = (int)arguments.size() - 1; i >= 0; i--) {
        int phase = findPhase(arguments[i], context);
        out << "mov eax, [esp - " << (((int)arguments.size() - i) * 4) << "]\n";
        out << "mov [ebp + " << phase << "], eax\n";
    }
}

void Dereference::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> dereference\n";
    arg->Compile(out, context);
    out << "mov eax, [esp - 4]\n";
    out << "mov ebx, [eax]\n";
    out << "mov [esp - 4], ebx\n";
}

void Addition::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> addition\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "add eax, [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Subtraction::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> subtraction\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Multiplication::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> multiplication\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "mov edx, [esp - 8]\n";
    out << "mul edx\n";
    out << "mov [esp - 4], eax\n";
}

void Division::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> division\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "mov edx, 0\n";
    out << "div dword [esp - 8]\n";
    out << "mov [esp - 4], eax\n";
}

void Less::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> less\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    int idx = context.branch_index++;
    out << "jl " << "_set1_" << idx << "\n";
    out << "mov [esp - 4], dword 0\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov [esp - 4], dword 1\n";
    out << "_setend" << idx << ":\n";
}

void Equal::Compile(std::ostream &out, CPContext &context) {
    out << "; " << filename << " " << line_begin + 1 << ":" << position_begin + 1 << " -> equal\n";
    left->Compile(out, context);
    out << "sub esp, 4\n";
    context.variable_stack.push_back("__junk");
    right->Compile(out, context);
    out << "add esp, 4\n";
    context.variable_stack.pop_back();
    out << "mov eax, [esp - 4]\n";
    out << "sub eax, [esp - 8]\n";
    int idx = context.branch_index++;
    out << "jz " << "_set1_" << idx << "\n";
    out << "mov [esp - 4], dword 0\n";
    out << "jmp _setend" << idx << "\n";
    out << "_set1_" << idx << ":\n";
    out << "mov [esp - 4], dword 1\n";
    out << "_setend" << idx << ":\n";
}*/

void CompileNode(struct Node *node, FILE *out, struct CPContext *context) {
    if (node->node_type == NodeBlock) {
        CompileBlock(node, out, context);
    }
    else if (node->node_type == NodeDefinition) {
        CompileDefinition(node, out, context);
    }
    else if (node->node_type == NodeAssignment) {
        CompileAssignment(node, out, context);
    }
    else if (node->node_type == NodeMovement) {
        CompileMovement(node, out, context);
    }
    else if (node->node_type == NodeAsm) {
        CompileAsm(node, out, context);
    }
    else if (node->node_type == NodeIf) {
        CompileIf(node, out, context);
    }
    else if (node->node_type == NodeWhile) {
        CompileWhile(node, out, context);
    }
}
