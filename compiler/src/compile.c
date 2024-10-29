#include <ast.h>
#include <process.h>
#include <compile.h>
#include <settings.h>
#include <vector.h>
#include <context.h>
#include <exception.h>
#include <posix.h>
#include <stdio.h>
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

struct Type *CompileNode(struct Node *node, struct CPContext *context);

void GenerateTestFunction(struct CPContext *context, struct Settings *settings) {
    _fputs(context->fd_text, "global test\n");
    _fputs(context->fd_text, "test:\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    for (int i = 0; i < vsize(&context->test_names); i++) {
        _fputs3(context->fd_text, "call ", context->test_names.ptr[i], "\n");
        _fputs(context->fd_text, "cmp rax, 0\n");
        _fputsi(context->fd_text, "jne _L", idx, "\n");
    }
    _fputs(context->fd_text, "mov rdi, 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov rdi, 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    _fputs(context->fd_text, "mov rax, 0x3c\n");
    _fputs(context->fd_text, "syscall\n");
//     int fd[2];
//     posix_pipe(fd);
// 
//     _fputs(fd[1], "func ^.run_tests() -> <int, 0> {\n");
//     for (int i = 0; i < vsize(&context->test_names); i++) {
//         _fputs3(fd[1], "if (.", context->test_names.ptr[i], "() = 0) {} else { return 1 }\n");
//     }
//     _fputs(fd[1], "return 0\n");
//     _fputs(fd[1], "}\n");
//     posix_close(fd[1]);
// 
//     struct Node *node = process_parse_fd(fd[0], settings);
//     posix_close(fd[0]);
//     return node;
}

void compile_process(struct Node *node, struct Settings *settings) {
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
    context->test_names = vnew();
    context->testing = settings->testing;
    context->header = false;
    
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

    _fputs(context->fd_bss, "section .bss\n");
    _fputs(context->fd_data, "section .data\n");
    
    _fputs(context->fd_text, "section .text\n");
    _fputs(context->fd_text, "; ");
    _fputs(context->fd_text, node->filename);
    _fputs(context->fd_text, " ");
    _fputi(context->fd_text, node->line_begin + 1);
    _fputs(context->fd_text, ":");
    _fputi(context->fd_text, node->position_begin + 1);
    _fputs(context->fd_text, " -> program\n");

    CompileNode(node, context);

    if (context->testing) {
        GenerateTestFunction(context, settings);
    }

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
    write_file(settings->filename_compile_output, program);
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
    _fputsi(context->fd_text, "add rsp, ", context->sf_pos - old_sf_pos, "\n");
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop(&context->function_name_front);
        vpop(&context->function_name_back);
        vpop(&context->function_signature);
    }
    context->sf_pos = old_sf_pos;
}

void CompileInclude(struct Node *node, struct Include *this, struct CPContext *context) {
    int old_header = context->header;
    context->header = true;
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        CompileNode(this->statement_list.ptr[i], context);
    }
    context->header = old_header;
}

void CompileTest(struct Node *node, struct Test *this, struct CPContext *context) {
    vpush(&context->test_names, (void*)this->name);
    char *identifier_end = concat("_T", this->name);

    if (context->header) {
        _fputs3(context->fd_text, "extern ", this->name, "\n");
        return;
    }

    _fputs3(context->fd_text, "jmp ", identifier_end, "\n");
    _fputs2(context->fd_text, this->name, ":\n");
    _fputs(context->fd_text, "push rbp\n");
    _fputs(context->fd_text, "mov rbp, rsp\n");

    struct Vector variables_tmp = context->variables;
    int sf_pos_tmp = context->sf_pos;
    context->variables = vnew();
    context->sf_pos = -8;

    CompileNode(this->block, context);
    
    vdrop(&context->variables);
    context->variables = variables_tmp;
    context->sf_pos = sf_pos_tmp;
    
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
    _fputs2(context->fd_text, identifier_end, ":\n");

    _free(identifier_end);
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
        _fputsi(context->fd_text, "_L", idx + i, ":\n");
        CompileNode(this->condition_list.ptr[i], context);
        _fputs(context->fd_text, "cmp qword [rsp - 8], 0\n");
        _fputsi(context->fd_text, "je _L", idx + i + 1, "\n");
        CompileNode(this->block_list.ptr[i], context);
        _fputsi(context->fd_text, "jmp _L", last, "\n");
    }
    _fputsi(context->fd_text, "_L", idx + sz, ":\n");
    if (this->else_block) {
        CompileNode(this->else_block, context);
        _fputsi(context->fd_text, "_L", idx + sz + 1, ":\n");
    }
}

void CompileWhile(struct Node *node, struct While *this, struct CPContext *context) {
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "_L", idx, ":\n");
    CompileNode(this->condition, context);
    _fputs(context->fd_text, "cmp qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "je _L", idx + 1, "\n");
    CompileNode(this->block, context);
    _fputsi(context->fd_text, "jmp _L", idx, "\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
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
        identifier_back = concat("_Z", _itoa(context->function_index));
        context->function_index++;
    }
    identifier_end = concat("_E", identifier_back);

    vpush(&context->function_name_front, _strdup(identifier_front));
    vpush(&context->function_name_back, _strdup(identifier_back));
    vpush(&context->function_signature, this->signature);

    if (this->external || this->struct_name) {
        if (context->header) {
            _fputs3(context->fd_text, "extern ", identifier_back, "\n");
            return;
        }
        else {
            _fputs3(context->fd_text, "global ", identifier_back, "\n");
        }
    }

    _fputs3(context->fd_text, "jmp ", identifier_end, "\n");
    _fputs2(context->fd_text, identifier_back, ":\n");
    _fputs(context->fd_text, "push rbp\n");
    _fputs(context->fd_text, "mov rbp, rsp\n");

    struct Vector variables_tmp = context->variables;
    int sf_pos_tmp = context->sf_pos;
    context->variables = vnew();
    context->sf_pos = -8;

    if (this->struct_name) {
        if (vsize(&this->signature->identifiers)) {
            struct Type *type = this->signature->types.ptr[0];
            if (_strcmp(type->identifier, this->struct_name) || type->degree != 1) {
                error_semantic("Pointer to struct expected as first argument in function method", node);
            }
        }
        else error_semantic("Pointer to struct expected as first argument in function method", node);
    }

    int sz = vsize(&this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        _fputs3(context->fd_text, "push ", regs[i], "\n");
        struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
        var_info->name = this->signature->identifiers.ptr[i];
        var_info->type = type_copy(this->signature->types.ptr[i]);
        var_info->sf_phase = context->sf_pos;
        context->sf_pos -= 8;
        vpush(&context->variables, var_info);
    }
    CompileNode(this->block, context);
    
    _fputsi(context->fd_text, "add rsp, ", sz * 8, "\n");
    for (int i = 0; i < sz; i++) {
        _free(context->variables.ptr[i]);
    }
    vdrop(&context->variables);
    context->variables = variables_tmp;
    context->sf_pos = sf_pos_tmp;
    
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
    _fputs2(context->fd_text, identifier_end, ":\n");
    
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
    _fputs3(context->fd_text, "extern ", identifier, "\n");
    vpush(&context->function_name_front, (void*)identifier);
    vpush(&context->function_name_back, (void*)identifier);
    vpush(&context->function_signature, this->signature);
}

void CompileStructDefinition(struct Node *node, struct StructDefinition *this, struct CPContext *context) {
    struct StructInfo *_struct = (struct StructInfo*)_malloc(sizeof(struct StructInfo));
    _struct->name = this->name;
    _struct->variables = vnew();
    _struct->size = 0;
    int sz = vsize(&this->identifiers);
    for (int i = 0; i < sz; i++) {
        struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
        var_info->name = _strdup(this->identifiers.ptr[i]);
        var_info->type = type_copy(this->types.ptr[i]);
        var_info->sf_phase = _struct->size;
        _struct->size += type_size(var_info->type, context, false);
        vpush(&_struct->variables, var_info);
    }
    vpush(&context->structs, (void*)_struct);
}

void CompileDefinition(struct Node *node, struct Definition *this, struct CPContext *context) {
    struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
    var_info->name = _strdup(this->identifier);
    var_info->type = type_copy(this->type);
    var_info->sf_phase = context->sf_pos;
    vpush(&context->variables, var_info);
    context->sf_pos -= type_size(this->type, context, true);

    _fputsi(context->fd_text, "sub rsp, ", type_size(this->type, context, true), "\n");
}

void CompileReturn(struct Node *node, struct Return *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
}

struct Type *CompileAs(struct Node *node, struct As *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->expression, context);
    _free(_type);
    return type_copy(this->type);
}

void CompileAssignment(struct Node *node, struct Assignment *this, struct CPContext *context) {
    if (this->dst->node_type != NodeIdentifier) {
        error_semantic("Identifier expected in assignment", node);
    }
    struct Identifier *_identifier = (struct Identifier*)(this->dst->node_ptr);
    struct VariableInfo *var_info = context_find_variable(context, _identifier->identifier);
    if (!var_info) {
        error_semantic("Variable was not declared in assignment", node);
    }
    struct Type *_type1 = var_info->type;
    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree) error_semantic("Assignment of not equal types", node);
    _free(_type2);

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputsi(context->fd_text, "mov [rbp + ", var_info->sf_phase, "], rax\n");
}

void CompileMovement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->dst, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in movement", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->src, context);
    if (_strcmp(_type1->identifier, _type2->identifier) || _type1->degree != _type2->degree + 1) {
        error_semantic("Movement of inappropriate types", node);
    }

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rbx, [rsp - 16]\n");
    if (!_strcmp(_type1->identifier, "char")) {
        _fputs(context->fd_text, "mov [rax], bl\n");
    }
    else {
        _fputs(context->fd_text, "mov [rax], rbx\n");
    }
    
    _free(_type1);
    _free(_type2);
}

struct Type *CompileIdentifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    struct VariableInfo *var_info = context_find_variable(context, this->identifier);
    if (!var_info) {
        error_semantic("Variable was not declared", node);
    }
    _fputsi(context->fd_text, "mov rax, [rbp + ", var_info->sf_phase, "]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");

    struct Type *type = var_info->type;
    return type_copy(type);
}

struct Type *CompileInteger(struct Node *node, struct Integer *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return type_build("int", 0);
}

struct Type *CompileChar(struct Node *node, struct Char *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return type_build("char", 0);
}

struct Type *CompileString(struct Node *node, struct String *this, struct CPContext *context) {
    int idx = context->data_index;
    context->data_index++;
    _fputsi(context->fd_data, "_S", idx, ":\n");
    _fputs(context->fd_data, "db ");
    for (int i = 0; i < _strlen(this->value); i++) {
        _fputi(context->fd_data, this->value[i]);
        _fputs(context->fd_data, ", ");
    }
    _fputs(context->fd_data, "0\n");
    _fputsi(context->fd_text, "mov qword [rsp - 8], _S", idx, "\n");
    return type_build("char", 1);
}

struct Type *CompileArray(struct Node *node, struct Array *this, struct CPContext *context) {
    int size = vsize(&this->values);
    if (size == 0) {
        error_semantic("Array has to be non-empty", node);
    }
    int idx = context->bss_index;
    context->bss_index++;
    struct Type *_type = NULL;
    _fputsi(context->fd_bss, "_B", idx, ":\n");
    _fputsi(context->fd_bss, "resb ", size * 8, "\n");
    for (int i = 0; i < size; i++) {
        struct Type *_type2 = CompileNode(this->values.ptr[i], context);
        if (i == 0) {
            _type = _type2;
            if (!(_type)) {
                error_semantic("Base type expected", node);
            }
        }
        if (!type_equal(_type, _type2)) {
            error_semantic("Array elements have to have same type", node);
        }
        if (i != 0) _free(_type2);
        _fputsi(context->fd_text, "mov rax, _B", idx, "\n");
        _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
        _fputsi(context->fd_text, "mov [rax + ", i * 8, "], rbx\n");
    }
    _fputsi(context->fd_text, "mov qword [rsp - 8], _B", idx, "\n");
    _type->degree++;
    return _type;
}

struct Type *CompileSizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    int size = type_size(this->type, context, false);
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", size, "\n");
    return type_build("int", 0);
}

struct Type *CompileFunctionCall(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    char *identifier;
    if (this->caller) {
        struct Type *_type = CompileNode(this->caller, context);
        identifier = concat(_type->identifier, this->identifier);
        _free(_type);
        _fputs3(context->fd_text, "mov ", regs[0], ", [rsp - 8]\n");
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
        error_semantic("Function was not declared", node);
    }

    struct FunctionSignature *signature = context->function_signature.ptr[function_idx];
    int sz = vsize(&this->arguments);
    bool has_caller = (this->caller != NULL);
    if (sz + has_caller != vsize(&signature->identifiers)) error_semantic("Incorrect number of arguments in function call", node);

    for (int i = 0; i < sz; i++) {
        struct Type *_type = CompileNode(this->arguments.ptr[i], context);
        if (!type_equal(_type, signature->types.ptr[i + has_caller])){
            error_semantic("Passing to function value of incorrect type", node);
        }
        _free(_type);
        _fputs(context->fd_text, "mov ");
        _fputs(context->fd_text, regs[i + has_caller]);
        _fputs(context->fd_text, ", [rsp - 8]\n");
    }
    
    const char *identifier_back = context->function_name_back.ptr[function_idx];
    _fputs3(context->fd_text, "call ", identifier_back, "\n");
    _free(identifier);
    
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    
    return type_copy(signature->return_type);
}

struct Type *CompileDereference(struct Node *node, struct Dereference *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->expression, context);
    if (_type->degree == 0) error_semantic("Dereference of not pointer value", node);
    _type->degree--;
    int _type_size = type_size(_type, context, false);

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    if (_type_size == 1) {
        _fputs(context->fd_text, "mov rbx, 0\n");
        _fputs(context->fd_text, "mov bl, [rax]\n");
    }
    else if (_type_size == 8) {
        _fputs(context->fd_text, "mov rbx, [rax]\n");
    }
    else {
        error_semantic("Not implemented", node);
    }
    _fputs(context->fd_text, "mov rbx, [rax]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");

    return _type;
}

struct Type *CompileGetField(struct Node *node, struct GetField *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree != 1) error_semantic("Pointer to structure expected", node);
    struct StructInfo *_struct = context_find_struct(context, _type->identifier);
    if (!_struct) error_semantic("Structure was not declared", node);
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
    if (!found) error_semantic("Structure doesn't have corresponding field", node);
    
    _type = type_copy(var_info->type);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    if (!this->address) {    
        _fputsi(context->fd_text, "mov rbx, [rax + ", var_info->sf_phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    else {
        _fputsi(context->fd_text, "lea rbx, [rax + ", var_info->sf_phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
        _type->degree++;
    }

    return _type;
}

struct Type *CompileIndex(struct Node *node, struct Index *this, struct CPContext *context) {
    struct Type *_type = CompileNode(this->left, context);
    if (_type->degree == 0) error_semantic("Pointer expected in indexation", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->right, context);
    if (_strcmp(_type2->identifier, "int") || _type2->degree != 0) error_semantic("Integer expected in indexation", node);
    _free(_type2);
    
    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _type->degree--;
    int _type_size = type_size(_type, context, false);
    _fputsi(context->fd_text, "mov rbx, ", _type_size, "\n");
    _type->degree++;
    _fputs(context->fd_text, "mul rbx\n");
    _fputs(context->fd_text, "add rax, [rsp - 8]\n");
    if (!this->address) {
        if (_type_size == 1) {
            _fputs(context->fd_text, "mov rbx, 0\n");
            _fputs(context->fd_text, "mov bl, [rax]\n");
        }
        else if (_type_size == 8) {
            _fputs(context->fd_text, "mov rbx, [rax]\n");
        }
        else {
            error_semantic("Not implemented", node);
        }
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
        _type->degree--;
    }
    else {
        _fputs(context->fd_text, "lea rbx, [rax]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    return _type;
}

struct Type *CompileArithmetic(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type1 = CompileNode(this->left, context);
    if (!type_is_int(_type1, context)) error_semantic("Integer expected in addition", this->left);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct Type *_type2 = CompileNode(this->right, context);
    if (!type_is_int(_type2, context)) error_semantic("Integer expected in addition", this->right);

    if (_strcmp(_type1->identifier, _type2->identifier)) error_semantic("Equal types expected in addition", node);
    _free(_type1);

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;

    return _type2;
}

struct Type *CompileAddition(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "add rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct Type *CompileSubtraction(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
}

struct Type *CompileMultiplication(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, [rsp - 16]\n");
    _fputs(context->fd_text, "mul rdx\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");

    return type_build("int", 0);
}

struct Type *CompileDivision(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, 0\n");
    _fputs(context->fd_text, "div qword [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
}

struct Type *CompileLess(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jl _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
}

struct Type *CompileEqual(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct Type *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jz _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
}

struct Type *CompileNode(struct Node *node, struct CPContext *context) {
    _fputs(context->fd_text, "; ");
    _fputs(context->fd_text, node->filename);
    _fputs(context->fd_text, " ");
    _fputi(context->fd_text, node->line_begin + 1);
    _fputs(context->fd_text, ":");
    _fputi(context->fd_text, node->position_begin + 1);
    _fputs(context->fd_text, " -> ");

    if (node->node_type == NodeBlock) {
        _fputs(context->fd_text, "block\n");
        CompileBlock(node, (struct Block*)node->node_ptr, context);
        return type_build("", 0);
    }
    if (node->node_type == NodeInclude) {
        _fputs(context->fd_text, "include\n");
        CompileInclude(node, (struct Include*)node->node_ptr, context);
        return type_build("", 0);
    }
    if (node->node_type == NodeTest) {
        _fputs(context->fd_text, "test\n");
        CompileTest(node, (struct Test*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeIf) {
        _fputs(context->fd_text, "if\n");
        CompileIf(node, (struct If*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeWhile) {
        _fputs(context->fd_text, "while\n");
        CompileWhile(node, (struct While*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        _fputs(context->fd_text, "function definition\n");
        CompileFunctionDefinition(node, (struct FunctionDefinition*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodePrototype) {
        _fputs(context->fd_text, "prototype\n");
        CompilePrototype(node, (struct Prototype*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeStructDefinition) {
        _fputs(context->fd_text, "struct definition\n");
        CompileStructDefinition(node, (struct StructDefinition*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeDefinition) {
        _fputs(context->fd_text, "definition\n");
        CompileDefinition(node, (struct Definition*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeReturn) {
        _fputs(context->fd_text, "return\n");
        CompileReturn(node, (struct Return*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeAs) {
        _fputs(context->fd_text, "as\n");
        return CompileAs(node, (struct As*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAssignment) {
        _fputs(context->fd_text, "assignment\n");
        CompileAssignment(node, (struct Assignment*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeMovement) {
        _fputs(context->fd_text, "movement\n");
        CompileMovement(node, (struct Movement*)node->node_ptr, context);
        return type_build("", 0);
    }
    else if (node->node_type == NodeIdentifier) {
        _fputs(context->fd_text, "identifier\n");
        return CompileIdentifier(node, (struct Identifier*)node->node_ptr, context);
    }
    else if (node->node_type == NodeInteger) {
        _fputs(context->fd_text, "integer\n");
        return CompileInteger(node, (struct Integer*)node->node_ptr, context);
    }
    else if (node->node_type == NodeChar) {
        _fputs(context->fd_text, "char\n");
        return CompileChar(node, (struct Char*)node->node_ptr, context);
    }
    else if (node->node_type == NodeString) {
        _fputs(context->fd_text, "string\n");
        return CompileString(node, (struct String*)node->node_ptr, context);
    }
    else if (node->node_type == NodeArray) {
        _fputs(context->fd_text, "array\n");
        return CompileArray(node, (struct Array*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSizeof) {
        _fputs(context->fd_text, "sizeof\n");
        return CompileSizeof(node, (struct Sizeof*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        _fputs(context->fd_text, "function call\n");
        return CompileFunctionCall(node, (struct FunctionCall*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDereference) {
        _fputs(context->fd_text, "dereference\n");
        return CompileDereference(node, (struct Dereference*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIndex) {
        _fputs(context->fd_text, "index\n");
        return CompileIndex(node, (struct Index*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGetField) {
        _fputs(context->fd_text, "get field\n");
        return CompileGetField(node, (struct GetField*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAddition) {
        _fputs(context->fd_text, "addition\n");
        return CompileAddition(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSubtraction) {
        _fputs(context->fd_text, "subtraction\n");
        return CompileSubtraction(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMultiplication) {
        _fputs(context->fd_text, "multiplication\n");
        return CompileMultiplication(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDivision) {
        _fputs(context->fd_text, "division\n");
        return CompileDivision(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLess) {
        _fputs(context->fd_text, "less\n");
        return CompileLess(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeEqual) {
        _fputs(context->fd_text, "equal\n");
        return CompileEqual(node, (struct BinaryOperator*)node->node_ptr, context);
    }
}
