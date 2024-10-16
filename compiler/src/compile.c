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

struct VariableInfo *findVariable(const char *identifier, struct CPContext *context) {
    int sz = vsize(&context->variables);
    for (int i = sz - 1; i >= 0; i--) {
        struct VariableInfo *var_info = context->variables.ptr[i];
        if (_strcmp(var_info->name, identifier) == 0) {
            return var_info;
        }
    }
    return NULL;
}

struct Struct *findStruct(const char *identifier, struct CPContext *context) {
    int sz = vsize(&context->structs);
    for (int i = 0; i < sz; i++) {
        struct Struct *_struct = context->structs.ptr[i];
        if (_strcmp(_struct->name, identifier) == 0) {
            return context->structs.ptr[i];
        }
    }
    return NULL;
}

bool isIntType(struct Type *type, struct CPContext *context) {
    if (type->degree != 0) {
        return false;
    }
    if (_strcmp(type->identifier, "int") == 0) {
        return true;
    }
    if (_strcmp(type->identifier, "char") == 0) {
        return true;
    }
    return false;
}

bool typesEqual(struct Type *type1, struct Type *type2) {
    return (!_strcmp(type1->identifier, type2->identifier) && type1->degree == type2->degree);
}

int typeSize(struct Type *type, struct CPContext *context, bool aligned) {
    if (type->degree != 0) {
        return 8;
    }
    if (_strcmp(type->identifier, "int") == 0) {
        return 8;
    }
    if (_strcmp(type->identifier, "char") == 0) {
        if (aligned) {
            return 8;
        }
        else {
            return 1;
        }
    }
    else {
        struct Struct *_struct = findStruct(type->identifier, context);
        return _struct->size;
    }
}

struct Type *CompileNode(struct Node *node, struct CPContext *context);

void Compile(struct Node *node, struct Settings *settings) {
    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variables = vnew();
    context->sf_pos = -8;
    context->function_name_front = vnew();
    context->function_name_back = vnew();
    context->function_signature = vnew();
    context->structs = vnew();
    context->function_index = 0;
    context->branch_index = 0;
    context->data_index = 0;
    context->bss_index = 0;
    
    int fd[2];

    posix_pipe(fd);
    context->fd_text = fd[1];
    int fd_text_out = fd[0];

    posix_pipe(fd);
    context->fd_data = fd[1];
    int fd_data_out = fd[0];

    posix_pipe(fd);
    context->fd_bss = fd[1];
    int fd_bss_out = fd[0];

    print_string(context->fd_bss, "section .bss\n");
    print_string(context->fd_data, "section .data\n");
    
    print_string(context->fd_text, "section .text\n");
    print_string(context->fd_text, "; ");
    print_string(context->fd_text, node->filename);
    print_string(context->fd_text, " ");
    print_int   (context->fd_text, node->line_begin + 1);
    print_string(context->fd_text, ":");
    print_int   (context->fd_text, node->position_begin + 1);
    print_string(context->fd_text, " -> program\n");

    CompileNode(node, context);

    print_string(context->fd_text, "leave\n");
    print_string(context->fd_text, "ret\n");

    posix_close(context->fd_text);
    posix_close(context->fd_data);
    posix_close(context->fd_bss);
    char *str_bss = read_file_descriptor(fd_bss_out);
    char *str_data = read_file_descriptor(fd_data_out);
    char *str_text = read_file_descriptor(fd_text_out);
    posix_close(fd_text_out);
    posix_close(fd_data_out);
    posix_close(fd_bss_out);
    char *tmp = concat(str_bss, str_data);
    _free(str_bss);
    _free(str_data);
    char *program = concat(tmp, str_text);
    _free(tmp);
    _free(str_text);
    write_file(settings->compileOutputFilename, program);
}

void CompileBlock(struct Node *node, struct Block *this, struct CPContext *context) {
    int old_cnt_var = vsize(&context->variables);
    int old_sf_pos = context->sf_pos;
    int old_cnt_functions = vsize(&context->function_name_front);
    
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        CompileNode(this->statement_list.ptr[i], context);
    }

    int cnt_var = vsize(&context->variables);
    int cnt_functions = vsize(&context->function_name_front);
    print_stringi(context->fd_text, "add rsp, ", context->sf_pos - old_sf_pos, "\n");
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop_back(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop_back(&context->function_name_front);
        vpop_back(&context->function_name_back);
        vpop_back(&context->function_signature);
    }
    context->sf_pos = old_sf_pos;
}

void CompileIf(struct Node *node, struct If *this, struct CPContext *context) {
    int sz = vsize(&this->condition_list);
    int idx = context->branch_index;
    context->branch_index += sz + 1;
    int last = idx + sz;
    if (this->else_block) {
        context->branch_index++;
        last++;
    }
    for (int i = 0; i < sz; i++) {
        print_stringi(context->fd_text, "_L", idx + i, ":\n");
        CompileNode(this->condition_list.ptr[i], context);
        print_string(context->fd_text, "cmp qword [rsp - 8], 0\n");
        print_stringi(context->fd_text, "je _L", idx + i + 1, "\n");
        CompileNode(this->block_list.ptr[i], context);
        print_stringi(context->fd_text, "jmp _L", last, "\n");
    }
    print_stringi(context->fd_text, "_L", idx + sz, ":\n");
    if (this->else_block) {
        CompileNode(this->else_block, context);
        print_stringi(context->fd_text, "_L", idx + sz + 1, ":\n");
    }
}

void CompileWhile(struct Node *node, struct While *this, struct CPContext *context) {
    int idx = context->branch_index;
    context->branch_index += 2;
    print_stringi(context->fd_text, "_L", idx, ":\n");
    CompileNode(this->condition, context);
    print_string(context->fd_text, "cmp qword [rsp - 8], 0\n");
    print_stringi(context->fd_text, "je _L", idx + 1, "\n");
    CompileNode(this->block, context);
    print_stringi(context->fd_text, "jmp _L", idx, "\n");
    print_stringi(context->fd_text, "_L", idx + 1, ":\n");
}

void CompileFunctionDefinition(struct Node *node, struct FunctionDefinition *this, struct CPContext *context) {
    char *identifier_front, *identifier_back, *identifier_end;
    if (this->struct_name) {
        identifier_front = concat(this->struct_name, this->name);
        identifier_back = concat(this->struct_name, this->name);
    }
    else if (this->external) {
        identifier_front = _strdup(this->name);
        identifier_back = _strdup(this->name);
    }
    else {
        identifier_front = _strdup(this->name);
        identifier_back = concat("_Z", to_string(context->function_index));
        context->function_index++;
    }
    identifier_end = concat("_E", identifier_back);

    if (this->external) {
        print_string3(context->fd_text, "global ", identifier_back, "\n");
    }
    print_string3(context->fd_text, "jmp ", identifier_end, "\n");
    print_string2(context->fd_text, identifier_back, ":\n");
    print_string(context->fd_text, "push rbp\n");
    print_string(context->fd_text, "mov rbp, rsp\n");

    struct Vector variables_tmp = context->variables;
    int sf_pos_tmp = context->sf_pos;
    context->variables = vnew();
    context->sf_pos = -8;

    vpush_back(&context->function_name_front, _strdup(identifier_front));
    vpush_back(&context->function_name_back, _strdup(identifier_back));
    vpush_back(&context->function_signature, this->signature);

    if (this->struct_name) {
        if (vsize(&this->signature->identifiers)) {
            struct Type *type = this->signature->types.ptr[0];
            if (_strcmp(type->identifier, this->struct_name) || type->degree != 1) {
                SemanticError("Pointer to struct expected as first argument in function method", node);
            }
        }
        else SemanticError("Pointer to struct expected as first argument in function method", node);
    }

    int sz = vsize(&this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        print_string3(context->fd_text, "push ", regs[i], "\n");
        struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
        var_info->name = this->signature->identifiers.ptr[i];
        var_info->type = CopyType(this->signature->types.ptr[i]);
        var_info->sf_phase = context->sf_pos;
        context->sf_pos -= 8;
        vpush_back(&context->variables, var_info);
    }
    CompileNode(this->block, context);
    
    print_stringi(context->fd_text, "add rsp, ", sz * 8, "\n");
    for (int i = 0; i < sz; i++) {
        _free(context->variables.ptr[i]);
    }
    vdrop(&context->variables);
    context->variables = variables_tmp;
    
    print_string(context->fd_text, "leave\n");
    print_string(context->fd_text, "ret\n");
    print_string2(context->fd_text, identifier_end, ":\n");
    
    _free(identifier_front);
    _free(identifier_back);
    _free(identifier_end);
}

void CompilePrototype(struct Node *node, struct Prototype *this, struct CPContext *context) {
    const char *identifier;
    if (this->struct_name) {
        identifier = concat(this->struct_name, this->name);
    }
    else {
        identifier = this->name;
    }
    print_string3(context->fd_text, "extern ", identifier, "\n");
    vpush_back(&context->function_name_front, (void*)identifier);
    vpush_back(&context->function_name_back, (void*)identifier);
    vpush_back(&context->function_signature, this->signature);
}

void CompileStructDefinition(struct Node *node, struct StructDefinition *this, struct CPContext *context) {
    struct Struct *_struct = (struct Struct*)_malloc(sizeof(struct Struct));
    _struct->name = this->name;
    _struct->variables = vnew();
    _struct->size = 0;
    int sz = vsize(&this->identifiers);
    for (int i = 0; i < sz; i++) {
        struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
        var_info->name = _strdup(this->identifiers.ptr[i]);
        var_info->type = CopyType(this->types.ptr[i]);
        var_info->sf_phase = _struct->size;
        _struct->size += typeSize(var_info->type, context, false);
        vpush_back(&_struct->variables, var_info);
    }
    vpush_back(&context->structs, (void*)_struct);
}

void CompileDefinition(struct Node *node, struct Definition *this, struct CPContext *context) {
    struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
    var_info->name = _strdup(this->identifier);
    var_info->type = CopyType(this->type);
    var_info->sf_phase = context->sf_pos;
    vpush_back(&context->variables, var_info);
    context->sf_pos -= typeSize(this->type, context, true);

    print_stringi(context->fd_text, "sub rsp, ", typeSize(this->type, context, true), "\n");
}

void CompileReturn(struct Node *node, struct Return *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "leave\n");
    print_string(context->fd_text, "ret\n");
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
    struct VariableInfo *var_info = findVariable(_identifier->identifier, context);
    if (!var_info) {
        SemanticError("Variable was not declared in assignment", node);
    }
    struct Type *_type1 = var_info->type;
    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree) SemanticError("Assignment of not equal types", node);
    _free(_type2);

    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_stringi(context->fd_text, "mov [rbp + ", var_info->sf_phase, "], rax\n");
}

void CompileMovement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->dst, context);
    if (_type1->degree == 0) SemanticError("Pointer expected in movement", node);

    print_string(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree + 1) {
        SemanticError("Movement of inappropriate types", node);
    }

    print_string(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "mov rbx, [rsp - 16]\n");
    if (!_strcmp(_type1->identifier, "char")) {
        print_string(context->fd_text, "mov [rax], bl\n");
    }
    else {
        print_string(context->fd_text, "mov [rax], rbx\n");
    }
    
    _free(_type1);
    _free(_type2);
}

struct Type *CompileIdentifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    struct VariableInfo *var_info = findVariable(this->identifier, context);
    if (!var_info) {
        SemanticError("Variable was not declared", node);
    }
    print_stringi(context->fd_text, "mov rax, [rbp + ", var_info->sf_phase, "]\n");
    print_string(context->fd_text, "mov [rsp - 8], rax\n");

    struct Type *type = var_info->type;
    return BuildType(type->identifier, type->degree);
}

struct Type *CompileInteger(struct Node *node, struct Integer *this, struct CPContext *context) {
    print_stringi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return BuildType("int", 0);
}

struct Type *CompileChar(struct Node *node, struct Char *this, struct CPContext *context) {
    print_stringi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return BuildType("char", 0);
}

struct Type *CompileString(struct Node *node, struct String *this, struct CPContext *context) {
    int idx = context->data_index;
    context->data_index++;
    print_stringi(context->fd_data, "_S", idx, ":\n");
    print_string(context->fd_data, "db ");
    for (int i = 0; i < _strlen(this->value); i++) {
        print_int(context->fd_data, this->value[i]);
        print_string(context->fd_data, ", ");
    }
    print_string(context->fd_data, "0\n");
    print_stringi(context->fd_text, "mov qword [rsp - 8], _S", idx, "\n");
    return BuildType("char", 1);
}

struct Type *CompileArray(struct Node *node, struct Array *this, struct CPContext *context) {
    int size = vsize(&this->values);
    if (size == 0) {
        SemanticError("Array has to be non-empty", node);
    }
    int idx = context->bss_index;
    context->bss_index++;
    struct Type *_type = NULL;
    print_stringi(context->fd_bss, "_B", idx, ":\n");
    print_stringi(context->fd_bss, "resb ", size * 8, "\n");
    for (int i = 0; i < size; i++) {
        struct Type *_type2 = CompileNode(this->values.ptr[i], context);
        if (i == 0) {
            _type = _type2;
            if (!(_type)) {
                SemanticError("Base type expected", node);
            }
        }
        if (!typesEqual(_type, _type2)) {
            SemanticError("Array elements have to have same type", node);
        }
        if (i != 0) _free(_type2);
        print_stringi(context->fd_text, "mov rax, _B", idx, "\n");
        print_string(context->fd_text, "mov rbx, [rsp - 8]\n");
        print_stringi(context->fd_text, "mov [rax + ", i * 8, "], rbx\n");
    }
    print_stringi(context->fd_text, "mov qword [rsp - 8], _B", idx, "\n");
    _type->degree++;
    return _type;
}

struct Type *CompileSizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    int size = typeSize(this->type, context, false);
    print_stringi(context->fd_text, "mov qword [rsp - 8], ", size, "\n");
    return BuildType("int", 0);
}

struct Type *CompileFunctionCall(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    char *identifier;
    if (this->caller) {
        struct Type *_type = CompileNode(this->caller, context);
        identifier = concat(_type->identifier, this->identifier);
        _free(_type);
        print_string3(context->fd_text, "mov ", regs[0], ", [rsp - 8]\n");
    }
    else {
        identifier = _strdup(this->identifier);
    }
    
    int function_idx = -1;
    for (int i = vsize(&context->function_name_front) - 1; i >= 0; i--) {
        if (_strcmp(context->function_name_front.ptr[i], identifier) == 0) {
            function_idx = i;
            break;
        }
    }
    if (function_idx == -1) {
        SemanticError("Function was not defined", node);
    }

    struct FunctionSignature *signature = context->function_signature.ptr[function_idx];
    int sz = vsize(&this->arguments);
    bool has_caller = (this->caller != NULL);
    if (sz + has_caller != vsize(&signature->identifiers)) SemanticError("Incorrect number of arguments in function call", node);

    for (int i = 0; i < sz; i++) {
        struct Type *_type = CompileNode(this->arguments.ptr[i], context);
        if (!typesEqual(_type, signature->types.ptr[i + has_caller])){
            SemanticError("Passing to function value of incorrect type", node);
        }
        _free(_type);
        print_string(context->fd_text, "mov ");
        print_string(context->fd_text, regs[i + has_caller]);
        print_string(context->fd_text, ", [rsp - 8]\n");
    }
    
    const char *identifier_back = context->function_name_back.ptr[function_idx];
    print_string3(context->fd_text, "call ", identifier_back, "\n");
    _free(identifier);
    
    print_string(context->fd_text, "mov [rsp - 8], rax\n");
    
    return CopyType(signature->return_type);
}

struct Type *CompileDereference(struct Node *node, struct Dereference *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->expression, context);
    if (_type->degree == 0) SemanticError("Dereference of not pointer value", node);
    _type->degree--;

    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "mov rbx, [rax]\n");
    print_string(context->fd_text, "mov [rsp - 8], rbx\n");

    return _type;
}

struct Type *CompileGetField(struct Node *node, struct GetField *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree != 1) SemanticError("Pointer to structure expected", node);
    struct Struct *_struct = findStruct(_type->identifier, context);
    if (!_struct) SemanticError("Structure was not declared", node);
    _free(_type);

    int sz = vsize(&_struct->variables);
    struct VariableInfo *var_info;
    bool found = false;
    for (int i = 0; i < sz; i++) {
        var_info = _struct->variables.ptr[i];
        if (_strcmp(this->field, var_info->name) == 0) {
            found = true;
            break;
        }
    }
    if (!found) SemanticError("Structure doesn't have corresponding field", node);
    
    _type = CopyType(var_info->type);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    if (!this->address) {    
        print_stringi(context->fd_text, "mov rbx, [rax + ", var_info->sf_phase, "]\n");
        print_string(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    else {
        print_stringi(context->fd_text, "lea rbx, [rax + ", var_info->sf_phase, "]\n");
        print_string(context->fd_text, "mov [rsp - 8], rbx\n");
        _type->degree++;
    }

    return _type;
}

struct Type *CompileIndex(struct Node *node, struct Index *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree == 0) SemanticError("Pointer expected in indexation", node);

    print_string(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->right, context);
    if (_strcmp(_type2->identifier, "int") || _type2->degree != 0) SemanticError("Integer expected in indexation", node);
    _free(_type2);
    
    print_string(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;
    print_string(context->fd_text, "mov rax, [rsp - 16]\n");
    _type->degree--;
    print_stringi(context->fd_text, "mov rbx, ", typeSize(_type, context, false), "\n");
    _type->degree++;
    print_string(context->fd_text, "mul rbx\n");
    print_string(context->fd_text, "add rax, [rsp - 8]\n");
    if (!this->address) {
        print_string(context->fd_text, "mov rbx, [rax]\n");
        print_string(context->fd_text, "mov [rsp - 8], rbx\n");
        _type->degree--;
    }
    else {
        print_string(context->fd_text, "lea rbx, [rax]\n");
        print_string(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    return _type;
}

struct Type *CompileArithmetic(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->left, context);
    if (!isIntType(_type1, context)) SemanticError("Integer expected in addition", this->left);

    print_string(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->right, context);
    if (!isIntType(_type2, context)) SemanticError("Integer expected in addition", this->right);

    if (_strcmp(_type1->identifier, _type2->identifier)) SemanticError("Equal types expected in addition", node);
    _free(_type1);

    print_string(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;

    return _type2;
}

struct Type *CompileAddition(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "add rax, [rsp - 16]\n");
    print_string(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct Type *CompileSubtraction(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "sub rax, [rsp - 16]\n");
    print_string(context->fd_text, "mov [rsp - 8], rax\n");
}

struct Type *CompileMultiplication(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "mov rdx, [rsp - 16]\n");
    print_string(context->fd_text, "mul rdx\n");
    print_string(context->fd_text, "mov [rsp - 8], rax\n");

    return BuildType("int", 0);
}

struct Type *CompileDivision(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "mov rdx, 0\n");
    print_string(context->fd_text, "div qword [rsp - 16]\n");
    print_string(context->fd_text, "mov [rsp - 8], rax\n");
}

struct Type *CompileLess(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    print_stringi(context->fd_text, "jl _L", idx, "\n");
    print_string(context->fd_text, "mov qword [rsp - 8], 0\n");
    print_stringi(context->fd_text, "jmp _L", idx + 1, "\n");
    print_stringi(context->fd_text, "_L", idx, ":\n");
    print_string(context->fd_text, "mov qword [rsp - 8], 1\n");
    print_stringi(context->fd_text, "_L", idx + 1, ":\n");
}

struct Type *CompileEqual(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    print_string(context->fd_text, "mov rax, [rsp - 8]\n");
    print_string(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    print_stringi(context->fd_text, "jz _L", idx, "\n");
    print_string(context->fd_text, "mov qword [rsp - 8], 0\n");
    print_stringi(context->fd_text, "jmp _L", idx + 1, "\n");
    print_stringi(context->fd_text, "_L", idx, ":\n");
    print_string(context->fd_text, "mov qword [rsp - 8], 1\n");
    print_stringi(context->fd_text, "_L", idx + 1, ":\n");
}

struct Type *CompileNode(struct Node *node, struct CPContext *context) {
    print_string(context->fd_text, "; ");
    print_string(context->fd_text, node->filename);
    print_string(context->fd_text, " ");
    print_int   (context->fd_text, node->line_begin + 1);
    print_string(context->fd_text, ":");
    print_int   (context->fd_text, node->position_begin + 1);
    print_string(context->fd_text, " -> ");

    if (node->node_type == NodeBlock) {
        print_string(context->fd_text, "block\n");
        CompileBlock(node, (struct Block*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeIf) {
        print_string(context->fd_text, "if\n");
        CompileIf(node, (struct If*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeWhile) {
        print_string(context->fd_text, "while\n");
        CompileWhile(node, (struct While*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        print_string(context->fd_text, "function definition\n");
        CompileFunctionDefinition(node, (struct FunctionDefinition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodePrototype) {
        print_string(context->fd_text, "prototype\n");
        CompilePrototype(node, (struct Prototype*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeStructDefinition) {
        print_string(context->fd_text, "struct definition\n");
        CompileStructDefinition(node, (struct StructDefinition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeDefinition) {
        print_string(context->fd_text, "definition\n");
        CompileDefinition(node, (struct Definition*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeReturn) {
        print_string(context->fd_text, "return\n");
        CompileReturn(node, (struct Return*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeAs) {
        print_string(context->fd_text, "as\n");
        return CompileAs(node, (struct As*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAssignment) {
        print_string(context->fd_text, "assignment\n");
        CompileAssignment(node, (struct Assignment*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeMovement) {
        print_string(context->fd_text, "movement\n");
        CompileMovement(node, (struct Movement*)node->node_ptr, context);
        return BuildType("", 0);
    }
    else if (node->node_type == NodeIdentifier) {
        print_string(context->fd_text, "identifier\n");
        return CompileIdentifier(node, (struct Identifier*)node->node_ptr, context);
    }
    else if (node->node_type == NodeInteger) {
        print_string(context->fd_text, "integer\n");
        return CompileInteger(node, (struct Integer*)node->node_ptr, context);
    }
    else if (node->node_type == NodeChar) {
        print_string(context->fd_text, "char\n");
        return CompileChar(node, (struct Char*)node->node_ptr, context);
    }
    else if (node->node_type == NodeString) {
        print_string(context->fd_text, "string\n");
        return CompileString(node, (struct String*)node->node_ptr, context);
    }
    else if (node->node_type == NodeArray) {
        print_string(context->fd_text, "array\n");
        return CompileArray(node, (struct Array*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSizeof) {
        print_string(context->fd_text, "sizeof\n");
        return CompileSizeof(node, (struct Sizeof*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        print_string(context->fd_text, "function call\n");
        return CompileFunctionCall(node, (struct FunctionCall*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDereference) {
        print_string(context->fd_text, "dereference\n");
        return CompileDereference(node, (struct Dereference*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIndex) {
        print_string(context->fd_text, "index\n");
        return CompileIndex(node, (struct Index*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGetField) {
        print_string(context->fd_text, "get field\n");
        return CompileGetField(node, (struct GetField*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAddition) {
        print_string(context->fd_text, "addition\n");
        return CompileAddition(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSubtraction) {
        print_string(context->fd_text, "subtraction\n");
        return CompileSubtraction(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMultiplication) {
        print_string(context->fd_text, "multiplication\n");
        return CompileMultiplication(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDivision) {
        print_string(context->fd_text, "division\n");
        return CompileDivision(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLess) {
        print_string(context->fd_text, "less\n");
        return CompileLess(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeEqual) {
        print_string(context->fd_text, "equal\n");
        return CompileEqual(node, (struct BinaryOperator*)node->node_ptr, context);
    }
}
