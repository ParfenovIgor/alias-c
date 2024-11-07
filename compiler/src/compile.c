#include <ast.h>
#include <type.h>
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

#define WORD 8

const char *regs[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};

struct TypeNode *CompileNode(struct Node *node, struct CPContext *context);

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
}

void compile_process(struct Node *node, struct Settings *settings) {
    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variables = vnew();
    context->types = vnew();
    context->functions = vnew();
    context->sf_pos = -8;
    context->function_index = 0;
    context->branch_index = 0;
    context->data_index = 0;
    context->bss_index = 0;
    context->test_names = vnew();
    context->testing = settings->testing;
    context->header = false;

    {
        context->node_int = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeInt *_type = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
        context->node_int->node_ptr = _type;
        context->node_int->node_type = TypeNodeInt;
        context->node_int->degree = 0;
    }
    {
        context->node_char = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeChar *_type = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
        context->node_char->node_ptr = _type;
        context->node_char->node_type = TypeNodeChar;
        context->node_char->degree = 0;
    }
    
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
    int old_cnt_functions = vsize(&context->functions);
    
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        CompileNode(this->statement_list.ptr[i], context);
    }

    int cnt_var = vsize(&context->variables);
    int cnt_functions = vsize(&context->functions);
    _fputsi(context->fd_text, "add rsp, ", old_sf_pos - context->sf_pos, "\n");
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop(&context->functions);
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

struct TypeNode *from_signature_to_type(struct FunctionSignature *signature) {
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeFunction *_type = (struct TypeFunction*)_malloc(sizeof(struct TypeFunction));
    type->node_ptr = _type;
    type->node_type = TypeNodeFunction;
    type->degree = 0;
    _type->types = vnew();
    int sz = vsize(&signature->types);
    for (int i = 0; i < sz; i++) {
        vpush(&_type->types, signature->types.ptr[i]);
    }
    _type->return_type = signature->return_type;
    return type;
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

    struct FunctionInfo *function_info = (struct FunctionInfo*)_malloc(sizeof(struct FunctionInfo));
    function_info->name_front = identifier_front;
    function_info->name_back = identifier_back;
    function_info->type = from_signature_to_type(this->signature);
    vpush(&context->functions, function_info);

    if (this->external) {
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
    context->sf_pos = -WORD;

    if (this->struct_name) {
        if (vsize(&this->signature->identifiers)) {
            struct TypeNode *type1 = this->signature->types.ptr[0];
            type1->degree--;
            struct TypeNode *type2 = context_find_type(context, this->struct_name)->type;
            if (!type2) {
                error_semantic("Type identifier was not declared in method", node);
            }
            if (!type_equal(type1, type2, context)) {
                error_semantic("Pointer to type expected as first argument in method", node);
            }
            type1->degree++;
        }
        else error_semantic("Pointer to type expected as first argument in method", node);
    }

    int sz = vsize(&this->signature->identifiers);
    for (int i = 0; i < sz; i++) {
        _fputs3(context->fd_text, "push ", regs[i], "\n");
        struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
        var_info->name = this->signature->identifiers.ptr[i];
        var_info->type = this->signature->types.ptr[i];
        var_info->sf_phase = context->sf_pos;
        context->sf_pos -= WORD;
        vpush(&context->variables, var_info);
    }
    CompileNode(this->block, context);
    
    _fputsi(context->fd_text, "add rsp, ", sz * WORD, "\n");
    for (int i = 0; i < sz; i++) {
        _free(context->variables.ptr[i]);
    }
    vdrop(&context->variables);
    context->variables = variables_tmp;
    context->sf_pos = sf_pos_tmp;
    
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
    _fputs2(context->fd_text, identifier_end, ":\n");
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

    struct FunctionInfo *function_info = (struct FunctionInfo*)_malloc(sizeof(struct FunctionInfo));
    function_info->name_front = identifier;
    function_info->name_back = identifier;
    function_info->type = from_signature_to_type(this->signature);
    vpush(&context->functions, function_info);
}

void CompileDefinition(struct Node *node, struct Definition *this, struct CPContext *context) {
    struct TypeNode *_type = CompileNode(this->value, context);
    struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
    var_info->name = _strdup(this->identifier);
    var_info->type = _type;
    var_info->sf_phase = context->sf_pos;
    vpush(&context->variables, var_info);
    context->sf_pos -= WORD;

    _fputsi(context->fd_text, "sub rsp, ", WORD, "\n");
}

void CompileTypeDefinition(struct Node *node, struct TypeDefinition *this, struct CPContext *context) {
    struct TypeInfo *type_info = (struct TypeInfo*)_malloc(sizeof(struct TypeInfo));
    type_info->name = _strdup(this->identifier);
    type_info->type = this->type;
    vpush(&context->types, type_info);
}

void CompileReturn(struct Node *node, struct Return *this, struct CPContext *context) {
    CompileNode(this->expression, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
}

struct TypeNode *CompileAs(struct Node *node, struct As *this, struct CPContext *context) {
    struct TypeNode *_type = CompileNode(this->expression, context);
    return this->type;
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
    struct TypeNode *_type1 = var_info->type;
    struct TypeNode *_type2 = CompileNode(this->src, context);
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Assignment of not equal types", node);
    }

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputsi(context->fd_text, "mov [rbp + ", var_info->sf_phase, "], rax\n");
}

void CompileMovement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct TypeNode *_type1 = CompileNode(this->dst, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in movement", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= WORD;

    struct TypeNode *_type2 = CompileNode(this->src, context);
    _type2->degree++;
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Movement of inappropriate types", node);
    }
    _type2->degree--;

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += WORD;
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rbx, [rsp - 16]\n");
    if (type_size(_type1, context) == 1) {
        _fputs(context->fd_text, "mov [rax], bl\n");
    }
    else {
        _fputs(context->fd_text, "mov [rax], rbx\n");
    }
}

struct TypeNode *CompileIdentifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    struct TypeNode *_type = NULL;
    struct FunctionInfo *function_info = context_find_function(context, this->identifier);
    if (function_info) {
        _type = function_info->type;
        _fputs3(context->fd_text, "mov rax, ", function_info->name_back, "\n");
    }
    else {
        struct VariableInfo *var_info = context_find_variable(context, this->identifier);
        if (var_info) {
            _type = var_info->type;
            _fputsi(context->fd_text, "mov rax, [rbp + ", var_info->sf_phase, "]\n");
        }
        else {
            error_semantic("Variable was not declared", node);
        }
    }
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");

    return _type;
}

struct TypeNode *CompileInteger(struct Node *node, struct Integer *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeInt *_type = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
    type->node_ptr = _type;
    type->node_type = TypeNodeInt;
    type->degree = 0;
    return type;
}

struct TypeNode *CompileChar(struct Node *node, struct Char *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeChar *_type = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
    type->node_ptr = _type;
    type->node_type = TypeNodeChar;
    type->degree = 0;
    return type;
}

struct TypeNode *CompileString(struct Node *node, struct String *this, struct CPContext *context) {
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
    
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeChar *_type = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
    type->node_ptr = _type;
    type->node_type = TypeNodeChar;
    type->degree = 1;
    return type;
}

struct TypeNode *CompileArray(struct Node *node, struct Array *this, struct CPContext *context) {
    int size = vsize(&this->values);
    if (size == 0) {
        error_semantic("Array has to be non-empty", node);
    }
    int idx = context->bss_index;
    context->bss_index++;
    struct TypeNode *_type = NULL;
    _fputsi(context->fd_bss, "_B", idx, ":\n");
    _fputsi(context->fd_bss, "resb ", size * 8, "\n");
    for (int i = 0; i < size; i++) {
        struct TypeNode *_type2 = CompileNode(this->values.ptr[i], context);
        if (i == 0) {
            _type = _type2;
        }
        if (!type_equal(_type, _type2, context)) {
            error_semantic("Array elements have to have same type", node);
        }
        _fputsi(context->fd_text, "mov rax, _B", idx, "\n");
        _fputs(context->fd_text, "mov rbx, [rsp - 8]\n");
        _fputsi(context->fd_text, "mov [rax + ", i * 8, "], rbx\n");
    }
    _fputsi(context->fd_text, "mov qword [rsp - 8], _B", idx, "\n");
    _type = type_copy_node(_type);
    _type->degree++;
    return _type;
}

struct TypeNode *CompileStructInstance(struct Node *node, struct StructInstance *this, struct CPContext *context) {
    struct TypeNode *type_node = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    type_node->degree = 0;
    
    struct TypeStruct *type = (struct TypeStruct*)_malloc(sizeof(struct TypeStruct));
    type_node->node_type = TypeNodeStruct;
    type_node->node_ptr = type;
    type->names = vnew();
    type->types = vnew();

    int sz = vsize(&this->names);
    int struct_size = 0;
    for (int i = sz - 1; i >= 0; i--) {
        struct TypeNode *_type = CompileNode(this->values.ptr[i], context);
        int field_size = type_size(_type, context);
        struct_size += field_size;
        context->sf_pos -= field_size;
        _fputsi(context->fd_text, "sub rsp, ", field_size, "\n");
        vpush(&type->names, this->names.ptr[i]);
        vpush(&type->types, _type);
    }
    context->sf_pos += struct_size;
    _fputsi(context->fd_text, "add rsp, ", struct_size, "\n");
    return type_node;
}

struct TypeNode *CompileSizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    int size = type_size(this->type, context);
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", size, "\n");
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeInt *_type = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
    type->node_ptr = _type;
    type->node_type = TypeNodeInt;
    type->degree = 0;
    return type;
}

struct TypeNode *CompileFunctionCall(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    struct TypeNode *type = CompileNode(this->function, context);
    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= WORD;

    type = type_get_function(type, context);
    if (!type) {
        error_semantic("Function expected in function call", node);
    }
    struct TypeFunction *_type = type->node_ptr;
    int sz = vsize(&_type->types);
    if (sz != vsize(&this->arguments)) {
        error_semantic("Incorrect number of arguments in function call", node);
    }

    for (int i = 0; i < sz; i++) {
        struct TypeNode *type_arg = CompileNode(this->arguments.ptr[i], context);
        if (!type_equal(type_arg, _type->types.ptr[i], context)) {
            error_semantic("Passing to function value of incorrect type", node);
        }
        _fputs(context->fd_text, "sub rsp, 8\n");
        context->sf_pos -= WORD;
    }
    _fputsi(context->fd_text, "add rsp, ", WORD * (sz + 1), "\n");
    context->sf_pos += WORD * (sz + 1);

    for (int i = 0; i < sz; i++) {
        _fputs3(context->fd_text, "mov ", regs[i], ", ");
        _fputsi(context->fd_text, "[rsp - ", WORD * (i + 2), "]\n");
    }

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "call rax\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    
    return _type->return_type;
}

struct TypeNode *CompileDereference(struct Node *node, struct Dereference *this, struct CPContext *context) {
    struct TypeNode *_type = CompileNode(this->expression, context);
    if (_type->degree == 0) error_semantic("Dereference of not pointer value", node);
    _type = type_copy_node(_type);
    _type->degree--;
    int _type_size = type_size(_type, context);

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

struct TypeNode *CompileGetField(struct Node *node, struct GetField *this, struct CPContext *context) {
    struct TypeNode *type = CompileNode(this->left, context);
    if (type->degree != 1 || type->node_type != TypeNodeStruct) {
        error_semantic("Pointer to structure expected", node);
    }
    struct TypeStruct *_type = type->node_ptr;

    int sz = vsize(&_type->names);
    int phase = 0;
    struct TypeNode *field_type = NULL;
    for (int i = 0; i < sz; i++) {
        if (!_strcmp(this->field, _type->names.ptr[i])) {
            field_type = _type->types.ptr[i];
            break;
        }
        phase += type_size(_type->types.ptr[i], context);
    }

    if (!field_type) {
        error_semantic("Structure doesn't have corresponding field", node);
    }
    
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    if (!this->address) {    
        _fputsi(context->fd_text, "mov rbx, [rax + ", phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    else {
        _fputsi(context->fd_text, "lea rbx, [rax + ", phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
        field_type = type_copy_node(field_type);
        field_type->degree++;
    }

    return field_type;
}

struct TypeNode *CompileIndex(struct Node *node, struct Index *this, struct CPContext *context) {
    struct TypeNode *_type1 = CompileNode(this->left, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in indexation", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct TypeNode *_type2 = CompileNode(this->right, context);
    if (!type_equal(_type2, context->node_int, context)) {
        error_semantic("Integer expected in indexation", node);
    }
    
    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _type1->degree--;
    int _type_size = type_size(_type1, context);
    _fputsi(context->fd_text, "mov rbx, ", _type_size, "\n");
    _type1->degree++;
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
        _type1 = type_copy_node(_type1);
        _type1->degree--;
    }
    else {
        _fputs(context->fd_text, "lea rbx, [rax]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    }
    return _type1;
}

struct TypeNode *CompileArithmetic(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {    
    struct TypeNode *_type1 = CompileNode(this->left, context);
    if (!type_equal(_type1, context->node_int, context) && 
        !type_equal(_type1, context->node_char, context)) {
        error_semantic("Integer type expected in arithmetic operation", this->left);
    }

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct TypeNode *_type2 = CompileNode(this->right, context);
    if (!type_equal(_type2, context->node_int, context) && 
        !type_equal(_type2, context->node_char, context)) {
        error_semantic("Integer type expected in addition", this->right);
    }

    if (!type_equal(_type1, _type2, context)) error_semantic("Equal types expected in addition", node);

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;

    return _type1;
}

struct TypeNode *CompileAddition(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "add rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *CompileSubtraction(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *CompileMultiplication(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, [rsp - 16]\n");
    _fputs(context->fd_text, "mul rdx\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *CompileDivision(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, 0\n");
    _fputs(context->fd_text, "div qword [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
}

struct TypeNode *CompileLess(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
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

struct TypeNode *CompileEqual(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = CompileArithmetic(node, this, context);
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

struct TypeNode *CompileNode(struct Node *node, struct CPContext *context) {
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
    }
    if (node->node_type == NodeInclude) {
        _fputs(context->fd_text, "include\n");
        CompileInclude(node, (struct Include*)node->node_ptr, context);
    }
    if (node->node_type == NodeTest) {
        _fputs(context->fd_text, "test\n");
        CompileTest(node, (struct Test*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIf) {
        _fputs(context->fd_text, "if\n");
        CompileIf(node, (struct If*)node->node_ptr, context);
    }
    else if (node->node_type == NodeWhile) {
        _fputs(context->fd_text, "while\n");
        CompileWhile(node, (struct While*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        _fputs(context->fd_text, "function definition\n");
        CompileFunctionDefinition(node, (struct FunctionDefinition*)node->node_ptr, context);
    }
    else if (node->node_type == NodePrototype) {
        _fputs(context->fd_text, "prototype\n");
        CompilePrototype(node, (struct Prototype*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDefinition) {
        _fputs(context->fd_text, "definition\n");
        CompileDefinition(node, (struct Definition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeTypeDefinition) {
        _fputs(context->fd_text, "type definition\n");
        CompileTypeDefinition(node, (struct TypeDefinition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeReturn) {
        _fputs(context->fd_text, "return\n");
        CompileReturn(node, (struct Return*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAs) {
        _fputs(context->fd_text, "as\n");
        return CompileAs(node, (struct As*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAssignment) {
        _fputs(context->fd_text, "assignment\n");
        CompileAssignment(node, (struct Assignment*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMovement) {
        _fputs(context->fd_text, "movement\n");
        CompileMovement(node, (struct Movement*)node->node_ptr, context);
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
    else if (node->node_type == NodeStructInstance) {
        _fputs(context->fd_text, "struct instance\n");
        return CompileStructInstance(node, (struct StructInstance*)node->node_ptr, context);
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
    return NULL;
}
