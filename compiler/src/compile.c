#include "token.h"
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

enum Register {
    REG_A,
    REG_B,
    REG_C,
    REG_D
};

const char *codegen_register(enum Register reg) {
    switch (reg) {
        case REG_A: return "rax";
        case REG_B: return "rbx";
        case REG_C: return "rcx";
        case REG_D: return "rdx";
        default: return NULL;
    }
}

void codegen_buffer_zero(const char *label, int size, struct CPContext *context) {
    _fputs2(context->fd_bss, label, ":\n");
    _fputsi(context->fd_bss, "resb ", size, "\n");
}

void codegen_buffer_int(const char *label, int value, struct CPContext *context) {
    _fputs2(context->fd_data, label, ":\n");
    _fputsi(context->fd_data, "dq ", value, "\n");
}

void codegen_buffer_string(const char *label, const char *str, struct CPContext *context) {
    _fputs2(context->fd_data, label, ":\n");
    _fputs(context->fd_data, "db ");
    for (int i = 0; i < _strlen(str); i++) {
        _fputi(context->fd_data, str[i]);
        _fputs(context->fd_data, ", ");
    }
    _fputs(context->fd_data, "0\n");
}

void codegen_label(const char *label, struct CPContext *context) {
    _fputs2(context->fd_text, label, ":\n");
}

void codegen_move_regreg(enum Register dst, enum Register src, struct CPContext *context) {
    _fputs3(context->fd_text, "mov ", codegen_register(dst), ", ");
    _fputs2(context->fd_text, codegen_register(src), "\n");
}

void codegen_move_labelreg(enum Register dst, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "mov ", codegen_register(dst), ", ");
    _fputs2(context->fd_text, label, "\n");
}

void codegen_jump(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "jmp ", label, "\n");
}

void codegen_jump_ifzero(enum Register reg, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "cmp ", codegen_register(reg), ", 0\n");
    _fputs3(context->fd_text, "je ", label, "\n");
}

void codegen_jump_ifnonzero(enum Register reg, const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "cmp ", codegen_register(reg), ", 0\n");
    _fputs3(context->fd_text, "jne ", label, "\n");
}

void codegen_extern(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "extern ", label, "\n");
}

void codegen_global(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "global ", label, "\n");
}

void codegen_stack_shift(int x, struct CPContext *context) {
    _fputsi(context->fd_text, "add rsp, ", x, "\n");
}

void codegen_stack_pushword(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", codegen_register(reg), "\n");
}

void codegen_stack_popword(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword ", codegen_register(reg), ", [rsp - 8]\n");
}

void codegen_stack_pushint(int n, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", _itoa(n), "\n");
}

void codegen_stack_pushlabel(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", label, "\n");
}

void codegen_stack_pushword_phase(enum Register reg, int phase, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp + ", phase, "], ");
    _fputs2(context->fd_text, codegen_register(reg), "\n");
}

void codegen_stack_popword_phase(enum Register reg, int phase, struct CPContext *context) {
    _fputs2(context->fd_text, "mov qword ", codegen_register(reg));
    _fputsi(context->fd_text, ", [rsp + ", phase, "]\n");
}

void codegen_function_prologue(struct CPContext *context) {
    _fputs(context->fd_text, "push rbp\n");
    _fputs(context->fd_text, "mov rbp, rsp\n");
}

void codegen_function_epilogue(struct CPContext *context) {
    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
}

void codegen_syscall(const char *func, const char *arg, struct CPContext *context) {
    _fputs3(context->fd_text, "mov rax, ", func, "\n");
    _fputs3(context->fd_text, "mov rdi, ", arg, "\n");
    _fputs(context->fd_text, "syscall\n");
}

void compile_memcpy(const char *dst, const char *src, int sz, struct CPContext *context) {
    if (sz == 1) {
        _fputs3(context->fd_text, "mov cl, ", src, "\n");
        _fputs3(context->fd_text, "mov ", dst, ", cl\n");
    }
    else if (sz == 8) {
        _fputs3(context->fd_text, "mov rcx, ", src, "\n");
        _fputs3(context->fd_text, "mov ", dst, ", rcx\n");
    }
    else {
        _fputs3(context->fd_text, "lea rdi, ", dst, "\n");
        _fputs3(context->fd_text, "lea rsi, ", src, "\n");
        _fputsi(context->fd_text, "mov rcx, ", sz, "\n");
        _fputs(context->fd_text, "rep movsb\n");
    }
}

void compile_memzero(const char *dst, int sz, struct CPContext *context) {
    if (sz == 1) {
        _fputs3(context->fd_text, "mov byte", dst, ", 0\n");
    }
    else if (sz == 8) {
        _fputs3(context->fd_text, "mov qword", dst, ", 0\n");
    }
    else {
        _fputs(context->fd_text, "mov rsi, rax\n");
        _fputs3(context->fd_text, "lea rdi, ", dst, "\n");
        _fputs(context->fd_text, "mov rax, 0\n");
        _fputsi(context->fd_text, "mov rcx, ", sz, "\n");
        _fputs(context->fd_text, "rep stosb\n");
        _fputs(context->fd_text, "mov rax, rsi\n");
    }
}

void codegen_from_stackframe_to_stack(int stackframe_phase, int size, struct CPContext *context) {    
    const char *src = _concat3("[rbp + ", _itoa(stackframe_phase), "]");
    const char *dst = _concat3("[rsp - ", _itoa(size), "]");
    compile_memcpy(dst, src, size, context);
}

void codegen_from_global_to_stack(const char *identifier, int size, struct CPContext *context) {    
    const char *src = _concat3("[", identifier, "]");
    const char *dst = _concat3("[rsp - ", _itoa(size), "]");
    compile_memcpy(dst, src, size, context);
}

void codegen_get_stackframe_position(enum Register reg, int stackframe_phase, struct CPContext *context) {
    _fputs3(context->fd_text, "lea ", codegen_register(reg), ", ");
    _fputsi(context->fd_text, "[rbp + ", stackframe_phase, "]\n");
}

const char *codegen_call_arguments_registers[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"
};

void codegen_call_arguments_push(int n, struct CPContext *context) {
    if (n > sizeof(codegen_call_arguments_registers) / sizeof(codegen_call_arguments_registers[0])) {
        posix_exit(3);
    }
     for (int i = 0; i < n; i++) {
        _fputs3(context->fd_text, "push ", codegen_call_arguments_registers[i], "\n");
    }
}

void codegen_call_arguments_restore(int n, struct CPContext *context) {
    if (n > sizeof(codegen_call_arguments_registers) / sizeof(codegen_call_arguments_registers[0])) {
        posix_exit(3);
    }
    for (int i = 0; i < n; i++) {
        _fputs3(context->fd_text, "mov ", codegen_call_arguments_registers[i], ", ");
        _fputsi(context->fd_text, "[rsp - ", WORD * (i + 2), "]\n");
    }
}

void codegen_call_reg(enum Register reg, struct CPContext *context) {
    _fputs3(context->fd_text, "call ", codegen_register(reg), "\n");
}

void codegen_call_label(const char *label, struct CPContext *context) {
    _fputs3(context->fd_text, "call ", label, "\n");
}

int align_to_word(int x) {
    return (x + WORD - 1) / WORD * WORD;
}

struct TypeNode *compile_node(struct Node *node, struct CPContext *context);

void generate_test_function(struct CPContext *context, struct Settings *settings) {
    codegen_global("test", context);
    codegen_label("test", context);
    int idx = context->branch_index;
    context->branch_index++;
    for (int i = 0; i < vsize(&context->test_names); i++) {
        codegen_call_label(context->test_names.ptr[i], context);
        codegen_jump_ifnonzero(REG_A, _concat("_L", _itoa(idx)), context);
    }
    codegen_syscall("0x3c", "0", context);
    codegen_label(_concat("_L", _itoa(idx)), context);
    codegen_syscall("0x3c", "1", context);
}

void compile_init_descriptors(struct Node *node, struct Settings *settings, struct CPContext *context) {
    int fd[2];

    posix_pipe(fd);
    context->fd_text = fd[1];
    context->fd_text_out = fd[0];

    posix_pipe(fd);
    context->fd_data = fd[1];
    context->fd_data_out = fd[0];

    posix_pipe(fd);
    context->fd_bss = fd[1];
    context->fd_bss_out = fd[0];

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
}

void compile_flush_descriptors(struct Node *node, struct Settings *settings, struct CPContext *context) {
    posix_close(context->fd_text);
    posix_close(context->fd_data);
    posix_close(context->fd_bss);
    char *str_bss = read_file_descriptor(context->fd_bss_out);
    char *str_data = read_file_descriptor(context->fd_data_out);
    char *str_text = read_file_descriptor(context->fd_text_out);
    posix_close(context->fd_text_out);
    posix_close(context->fd_data_out);
    posix_close(context->fd_bss_out);
    char *program = _concat(_concat(str_bss, str_data), str_text);
    if (settings->filename_compile_output) {
        write_file(settings->filename_compile_output, program);
    }   
}

struct CPContext *compile_init_context(struct Settings *settings) {
    struct CPContext *context = (struct CPContext*)_malloc(sizeof(struct CPContext));
    context->variables = vnew();
    context->global_variables = vnew();
    context->types = vnew();
    context->functions = vnew();
    context->block_labels = vnew();
    context->loop_labels = vnew();
    context->sf_pos = 0;
    context->function_index = 0;
    context->branch_index = 0;
    context->data_index = 0;
    context->bss_index = 0;
    context->test_names = vnew();
    context->testing = settings->testing;
    context->header = false;

    {
        context->node_void = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeVoid *_type = (struct TypeVoid*)_malloc(sizeof(struct TypeVoid));
        context->node_void->node_ptr = _type;
        context->node_void->node_type = TypeNodeVoid;
        context->node_void->degree = 0;
    }
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
    {
        context->node_allocator = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
        struct TypeIdentifier *_type = (struct TypeIdentifier*)_malloc(sizeof(struct TypeIdentifier));
        context->node_allocator->node_ptr = _type;
        context->node_allocator->node_type = TypeNodeIdentifier;
        context->node_allocator->degree = 1;
        _type->identifier = _strdup("TestAllocator");
    }
    return context;
}

void compile_process(struct Node *node, struct Settings *settings) {
    struct CPContext *context = compile_init_context(settings);
    compile_init_descriptors(node, settings, context);
    compile_node(node, context);
    if (context->testing) {
        generate_test_function(context, settings);
    }
    compile_flush_descriptors(node, settings, context);
}

void compile_module(struct Node *node, struct Module *this, struct CPContext *context) {
    int old_cnt_var = vsize(&context->variables);
    int old_cnt_functions = vsize(&context->functions);

    for (int i = 0; i < vsize(&this->statement_list); i++) {
        compile_node(this->statement_list.ptr[i], context);
    }

    int cnt_var = vsize(&context->variables);
    int cnt_functions = vsize(&context->functions);
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop(&context->functions);
    }
}

struct TypeNode *compile_block(struct Node *node, struct Block *this, struct CPContext *context) {
    struct LabelInfo *label_info = (struct LabelInfo*)_malloc(sizeof(struct LabelInfo));
    if(this->label) label_info->name = this->label;
    else label_info->name = NULL;
    int idx = context->branch_index;
    context->branch_index += 2;
    label_info->name_begin = _concat("_L", _itoa(idx));
    label_info->name_end = _concat("_L", _itoa(idx + 1));
    label_info->type = NULL;
    label_info->sf_pos = context->sf_pos;
    vpush(&context->block_labels, label_info);
    codegen_label(label_info->name_begin, context);

    int old_cnt_var = vsize(&context->variables);
    int old_cnt_functions = vsize(&context->functions);
    
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        compile_node(this->statement_list.ptr[i], context);
    }

    int cnt_var = vsize(&context->variables);
    int cnt_functions = vsize(&context->functions);
    codegen_stack_shift(label_info->sf_pos - context->sf_pos, context);
    context->sf_pos = label_info->sf_pos;
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop(&context->functions);
    }
    codegen_label(label_info->name_end, context);

    struct TypeNode *type;
    if (!label_info->type) {
        type = context->node_void;
    }
    else {
        type = label_info->type;
    }
    vpop(&context->block_labels);
    return type;
}

void compile_include(struct Node *node, struct Include *this, struct CPContext *context) {
    int old_header = context->header;
    context->header = true;
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        compile_node(this->statement_list.ptr[i], context);
    }
    context->header = old_header;
}

struct TypeNode *compile_function_signature(struct Node *node, struct FunctionSignature *this, struct CPContext *context, struct Node *block, const char *identifier_back, const char *identifier_end) {
    codegen_jump(identifier_end, context);
    codegen_label(identifier_back, context);
    codegen_function_prologue(context);

    struct Vector variables_tmp = context->variables;
    context->variables = vnew();
    int sf_pos_tmp = context->sf_pos;
    context->sf_pos = 0;

    int sz = 0;
    if (this) {
        sz = vsize(&this->identifiers);
        if (!type_check(this->return_type, context)) {
            error_semantic("Type identifier was not declared in function signature return type", node);
        }
        for (int i = 0; i < sz; i++) {
            if (!type_check(this->types.ptr[i], context)) {
                error_semantic("Type identifier was not declared in function signature argument type", node);
            }
        }

        sz += (this->propagate_allocator != NULL);

        codegen_call_arguments_push(sz, context);
        for (int i = 0; i < sz; i++) {
            struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
            if (this->propagate_allocator) {
                if (i == 0) {
                    var_info->name = "@";
                    var_info->type = context->node_allocator;
                }
                else {
                    var_info->name = this->identifiers.ptr[i - 1];
                    var_info->type = this->types.ptr[i - 1];
                }
            }
            else {
                var_info->name = this->identifiers.ptr[i];
                var_info->type = this->types.ptr[i];
            }
            int size = align_to_word(type_size(var_info->type, context));
            var_info->sf_phase = context->sf_pos - size;
            context->sf_pos -= size;
            vpush(&context->variables, var_info);
        }
    }

    struct TypeNode *_type = compile_node(block, context);
    
    codegen_stack_popword(REG_A, context);
    codegen_stack_shift(-context->sf_pos, context);
    for (int i = 0; i < sz; i++) {
        _free(context->variables.ptr[i]);
    }
    vdrop(&context->variables);
    context->variables = variables_tmp;
    context->sf_pos = sf_pos_tmp;

    codegen_function_epilogue(context);
    codegen_label(identifier_end, context);

    return _type;
}

void compile_test(struct Node *node, struct Test *this, struct CPContext *context) {
    vpush(&context->test_names, (void*)this->name);
    char *identifier_end = _concat("_T", this->name);

    if (context->header) {
        codegen_extern(this->name, context);
        return;
    }

    compile_function_signature(node, NULL, context, this->block, this->name, identifier_end);
}

struct TypeNode *compile_if(struct Node *node, struct If *this, struct CPContext *context) {
    int sz = vsize(&this->condition_list);
    int idx = context->branch_index;
    context->branch_index += sz + 1;
    int last = idx + sz;
    if (this->else_block) {
        context->branch_index++;
        last++;
    }
    struct TypeNode *_type = NULL;
    for (int i = 0; i < sz; i++) {
        codegen_label(_concat("_L", _itoa(idx + i)), context);
        compile_node(this->condition_list.ptr[i], context);
        codegen_stack_popword(REG_A, context);
        codegen_jump_ifzero(REG_A, _concat("_L", _itoa(idx + i + 1)), context);
        struct TypeNode *_type2 = compile_node(this->block_list.ptr[i], context);
        if (_type && !type_equal(_type, _type2, context)) {
            error_semantic("If branches types do not equal", node);
        }
        if (!_type) _type = _type2;
        codegen_jump(_concat("_L", _itoa(last)), context);
    }
    codegen_label(_concat("_L", _itoa(idx + sz)), context);
    if (this->else_block) {
        struct TypeNode *_type2 = compile_node(this->else_block, context);
        if (_type && !type_equal(_type, _type2, context)) {
            error_semantic("If branches types do not equal", node);
        }
        codegen_label(_concat("_L", _itoa(idx + sz + 1)), context);
    }
    else if (!type_equal(_type, context->node_void, context)){
        error_semantic("If that returns non-void has to have else block", node);
    }
    return _type;
}

struct TypeNode *compile_while(struct Node *node, struct While *this, struct CPContext *context) {
    struct LabelInfo *label_info = (struct LabelInfo*)_malloc(sizeof(struct LabelInfo));
    if(this->label) label_info->name = this->label;
    else label_info->name = NULL;
    int idx = context->branch_index;
    context->branch_index += 3;
    label_info->name_begin = _concat("_L", _itoa(idx));
    label_info->name_end = _concat("_L", _itoa(idx + 2));
    label_info->type = NULL;
    label_info->sf_pos = context->sf_pos;
    vpush(&context->loop_labels, label_info);
    codegen_label(_concat("_L", _itoa(idx)), context);
    compile_node(this->condition, context);
    codegen_stack_popword(REG_A, context);
    codegen_jump_ifzero(REG_A, _concat("_L", _itoa(idx + 1)), context);
    compile_node(this->block, context);
    codegen_jump(_concat("_L", _itoa(idx)), context);
    codegen_label(_concat("_L", _itoa(idx + 1)), context);
    struct TypeNode *type_else = NULL;
    if (this->else_block) {
        type_else = compile_node(this->else_block, context);
    }
    codegen_label(_concat("_L", _itoa(idx + 2)), context);

    struct TypeNode *type;
    if ((!label_info->type || type_equal(label_info->type, context->node_void, context)) && 
        (!this->else_block || type_equal(type_else, context->node_void, context))) {
        type = context->node_void;
    }
    else if (label_info->type && this->else_block &&
            type_equal(label_info->type, type_else, context)) {
        type = label_info->type;
    }
    else if (label_info->type && !this->else_block) {
        return label_info->type;
    }
    else if(!label_info->type && this->else_block) {
        return type_else;
    }
    else {
        error_semantic("While branches types do not equal", node);
    }
    vpop(&context->loop_labels);

    return type;
}

struct TypeNode *from_signature_to_type(struct FunctionSignature *signature, struct CPContext *context) {
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeFunction *_type = (struct TypeFunction*)_malloc(sizeof(struct TypeFunction));
    type->node_ptr = _type;
    type->node_type = TypeNodeFunction;
    type->degree = 0;
    _type->types = vnew();
    if (signature->propagate_allocator) {
        vpush(&_type->types, context->node_allocator);
    }
    int sz = vsize(&signature->types);
    for (int i = 0; i < sz; i++) {
        vpush(&_type->types, signature->types.ptr[i]);
    }
    _type->return_type = signature->return_type;
    _type->propagate_allocator = (signature->propagate_allocator != NULL);
    return type;
}

void compile_function_definition(struct Node *node, struct FunctionDefinition *this, struct CPContext *context) { 
    if (this->caller_type &&  !type_check(this->caller_type, context)) {
        error_semantic("Type identifier was not declared in function caller type", node);
    }

    const char *identifier_front, *identifier_back, *identifier_end;
    if (this->external) {
        if (this->caller_type) {
            identifier_front = this->name;
            identifier_back = _concat(type_mangle(this->caller_type, context), this->name);
        }
        else {
            identifier_front = this->name;
            identifier_back = this->name;
        }
    }
    else {
        identifier_front = _strdup(this->name);
        identifier_back = _concat("_Z", _itoa(context->function_index));
        context->function_index++;
    }
    identifier_end = _concat("_E", identifier_back);

    struct FunctionInfo *function_info = (struct FunctionInfo*)_malloc(sizeof(struct FunctionInfo));
    function_info->name_front = identifier_front;
    function_info->name_back = identifier_back;
    function_info->caller_type = this->caller_type;
    function_info->type = from_signature_to_type(this->signature, context);
    vpush(&context->functions, function_info);

    if (this->external) {
        if (context->header) {
            codegen_extern(identifier_back, context);
            return;
        }
        else {
            codegen_global(identifier_back, context);
        }
    }

    if (this->caller_type) {
        if (vsize(&this->signature->identifiers)) {
            if (!type_equal(this->signature->types.ptr[0], this->caller_type, context)) {
                error_semantic("Caller type and first argument type in method have to be equal", node);
            }
        }
        else {
            error_semantic("Caller type and first argument type in method have to be equal", node);
        }
    }

    struct TypeNode *_type = compile_function_signature(node, this->signature, context, this->block, identifier_back, identifier_end);
    if (!type_equal(_type, this->signature->return_type, context)) {
        error_semantic("Function return type does not equal to the type of the function body", node);
    }
}

void compile_prototype(struct Node *node, struct Prototype *this, struct CPContext *context) {
    if (this->caller_type &&  !type_check(this->caller_type, context)) {
        error_semantic("Type identifier was not declared in function caller type", node);
    }

    const char *identifier;
    if (this->caller_type) {
        identifier = _concat(type_mangle(this->caller_type, context), this->name);
    }
    else{
        identifier = this->name;
    }
    codegen_extern(identifier, context);

    struct FunctionInfo *function_info = (struct FunctionInfo*)_malloc(sizeof(struct FunctionInfo));
    function_info->name_front = identifier;
    function_info->name_back = identifier;
    function_info->type = from_signature_to_type(this->signature, context);
    function_info->caller_type = this->caller_type;
    vpush(&context->functions, function_info);

    if (this->caller_type) {
        if (vsize(&this->signature->identifiers)) {
            if (!type_equal(this->signature->types.ptr[0], this->caller_type, context)) {
                error_semantic("Caller type and first argument type in method have to be equal", node);
            }
        }
        else {
            error_semantic("Caller type and first argument type in method have to be equal", node);
        }
    }
}

void compile_global_definition(struct Node *node, struct GlobalDefinition *this, struct CPContext *context) {
    struct TypeNode *_type;
    if (this->value) {
        _type = context->node_int;
        if (this->value->node_type != NodeInteger) {
            error_semantic("Const integer expected as initial value in global variable", node);
        }
        if (this->type && !type_equal(_type, this->type, context)) {
            error_semantic("Declared and inferred types are not equal", node);
        }
    }
    else {
        if (this->type) {
            _type = this->type;
        }
        else {
            error_semantic("Undefined type in definition", node);
        }
    }

    struct GlobalVariableInfo *global_var_info = (struct GlobalVariableInfo*)_malloc(sizeof(struct GlobalVariableInfo));
    global_var_info->name = _strdup(this->identifier);
    global_var_info->type = _type;
    vpush(&context->global_variables, global_var_info);

    if (this->value) {
        codegen_buffer_int(this->identifier, ((struct Integer*)(this->value->node_ptr))->value, context);
    }
    else {
        codegen_buffer_zero(this->identifier, type_size(this->type, context), context);
    }
}

void compile_definition(struct Node *node, struct Definition *this, struct CPContext *context) {
    if (this->type && !type_check(this->type, context)) {
        error_semantic("Type identifier was not declared in definition type", node);
    }

    struct TypeNode *_type;
    if (this->type && this->value) {
        _type = compile_node(this->value, context);
        if (this->type && !type_equal(_type, this->type, context)) {
            error_semantic("Declared and inferred types are not equal", node);
        }
    }
    if (this->type && !this->value) {
        _type = this->type;
    }
    if (!this->type && this->value) {
        _type = compile_node(this->value, context);
    }
    if (!this->type && !this->value) {
        error_semantic("Undefined type in definition", node);
    }

    struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
    var_info->name = _strdup(this->identifier);
    var_info->type = _type;
    int sz = align_to_word(type_size(_type, context));
    var_info->sf_phase = context->sf_pos - sz;
    vpush(&context->variables, var_info);
    context->sf_pos -= sz;
    codegen_stack_shift(-sz, context);
}

void compile_type_definition(struct Node *node, struct TypeDefinition *this, struct CPContext *context) {
    if (this->type &&  !type_check(this->type, context)) {
        error_semantic("Type identifier was not declared in type definition type", node);
    }
    
    struct TypeInfo *type_info = (struct TypeInfo*)_malloc(sizeof(struct TypeInfo));
    type_info->name = _strdup(this->identifier);
    type_info->type = this->type;
    vpush(&context->types, type_info);
}

void compile_return(struct Node *node, struct Return *this, struct CPContext *context) {
    struct TypeNode *_type = compile_node(this->expression, context);
    struct LabelInfo *label_info = context_find_block_label(context, this->label);
    if (!label_info) {
        error_semantic("Label was not declared", node);
    }
    if (!label_info->type) {
        label_info->type = _type;
    }
    else {
        if (!type_equal(_type, label_info->type, context)) {
            error_semantic("Return values types do not equal", node);
        }
    }

    int tsize = type_size(_type, context);
    const char *dst = _concat3("[rbp + ", _itoa(label_info->sf_pos - align_to_word(tsize)), "]");
    const char *src = _concat3("[rbp + ", _itoa(context->sf_pos - align_to_word(tsize)), "]");
    compile_memcpy(dst, src, tsize, context);

    codegen_stack_shift(label_info->sf_pos - context->sf_pos, context);
    codegen_jump(label_info->name_end, context);
}

void compile_break(struct Node *node, struct Break *this, struct CPContext *context) {
    struct TypeNode *_type = compile_node(this->expression, context);
    struct LabelInfo *label_info = context_find_loop_label(context, this->label);
    if (!label_info) {
        error_semantic("Label was not declared", node);
    }
    if (!label_info->type) {
        label_info->type = _type;
    }
    else {
        if (!type_equal(_type, label_info->type, context)) {
            error_semantic("Break values types do not equal", node);
        }
    }

    int tsize = type_size(_type, context);
    const char *dst = _concat3("[rbp + ", _itoa(label_info->sf_pos - align_to_word(tsize)), "]");
    const char *src = _concat3("[rbp + ", _itoa(context->sf_pos - align_to_word(tsize)), "]");
    compile_memcpy(dst, src, tsize, context);

    codegen_stack_shift(label_info->sf_pos - context->sf_pos, context);
    codegen_jump(label_info->name_end, context);
}

void compile_continue(struct Node *node, struct Continue *this, struct CPContext *context) {
    struct LabelInfo *label_info = context_find_loop_label(context, this->label);
    if (!label_info) {
        error_semantic("Label was not declared", node);
    }
    codegen_stack_shift(label_info->sf_pos - context->sf_pos, context);
    codegen_jump(label_info->name_begin, context);
}

struct TypeNode *compile_as(struct Node *node, struct As *this, struct CPContext *context) {
    if (!type_check(this->type, context)) {
        error_semantic("Type identifier was not declared in as type", node);
    }
    struct TypeNode *_type = compile_node(this->expression, context);
    return this->type;
}

void compile_assignment(struct Node *node, struct Assignment *this, struct CPContext *context) {
    if (this->dst->node_type != NodeIdentifier) {
        error_semantic("Identifier expected in assignment", node);
    }
    struct Identifier *_identifier = (struct Identifier*)(this->dst->node_ptr);
    struct VariableInfo *var_info = context_find_variable(context, _identifier->identifier);
    struct GlobalVariableInfo *global_var_info = context_find_global_variable(context, _identifier->identifier);
    if (!var_info && !global_var_info) {
        error_semantic("Variable was not declared in assignment", node);
    }
    struct TypeNode *_type1;
    if (var_info) {
        _type1 = var_info->type;
    }
    else {
        _type1 = global_var_info->type;
    }
    struct TypeNode *_type2 = compile_node(this->src, context);
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Assignment of not equal types", node);
    }

    int tsize = type_size(_type1, context);
    const char *dst;
    if (var_info) {
        dst = _concat3("[rbp + ", _itoa(var_info->sf_phase), "]");
    }
    else {
        dst = _concat3("[", global_var_info->name, "]");
    }
    const char *src = _concat3("[rsp - ", _itoa(align_to_word(tsize)), "]");
    compile_memcpy(dst, src, tsize, context);
}

void compile_movement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct TypeNode *_type1 = compile_node(this->dst, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in movement", node);

    codegen_stack_shift(-8, context);
    context->sf_pos -= WORD;

    struct TypeNode *_type2 = compile_node(this->src, context);
    _type2->degree++;
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Movement of inappropriate types", node);
    }
    _type2->degree--;

    codegen_stack_shift(8, context);
    context->sf_pos += WORD;

    int tsize = align_to_word(type_size(_type2, context));
    codegen_stack_popword(REG_A, context);
    const char *dst = "[rax]";
    const char *src = _concat3("[rsp - ", _itoa(tsize + WORD), "]");
    compile_memcpy(dst, src, type_size(_type2, context), context);
}

struct TypeNode *compile_identifier(struct Node *node, struct Identifier *this, struct CPContext *context) {
    struct TypeNode *_type = NULL;
    struct FunctionInfo *function_info = context_find_function(context, this->identifier);
    if (function_info) {
        if (this->address) {
            error_semantic("Can't take address of function", node);
        }
        _type = function_info->type;
        codegen_move_labelreg(REG_A, function_info->name_back, context);
        codegen_stack_pushword(REG_A, context);
        return _type;
    }
    struct VariableInfo *var_info = context_find_variable(context, this->identifier);
    struct GlobalVariableInfo *global_var_info = context_find_global_variable(context, this->identifier);
    if (var_info) {
        _type = type_copy_node(var_info->type);
    }
    else if (global_var_info) {
        _type = type_copy_node(global_var_info->type);
    }
    else {
        error_semantic("Identifier was not declared", node);
    }
    if (this->address) {
        if (var_info) {
            codegen_get_stackframe_position(REG_A, var_info->sf_phase, context);
            codegen_stack_pushword(REG_A, context);
        }
        else {
            codegen_stack_pushlabel(global_var_info->name, context);
        }
        _type->degree++;
    }
    else {
        if (var_info) {
            codegen_from_stackframe_to_stack(var_info->sf_phase, align_to_word(type_size(_type, context)), context);
        }
        else {
            codegen_from_global_to_stack(global_var_info->name, align_to_word(type_size(_type, context)), context);
        }
    }
    return _type;
}

struct TypeNode *compile_integer(struct Node *node, struct Integer *this, struct CPContext *context) {
    codegen_stack_pushint(this->value, context);
    return context->node_int;
}

struct TypeNode *compile_char(struct Node *node, struct Char *this, struct CPContext *context) {
    codegen_stack_pushint(this->value, context);
    return context->node_char;
}

struct TypeNode *compile_string(struct Node *node, struct String *this, struct CPContext *context) {
    int idx = context->data_index;
    context->data_index++;
    codegen_buffer_string(_concat("_S", _itoa(idx)), this->value, context);
    codegen_stack_pushlabel(_concat("_S", _itoa(idx)), context);
    
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeChar *_type = (struct TypeChar*)_malloc(sizeof(struct TypeChar));
    type->node_ptr = _type;
    type->node_type = TypeNodeChar;
    type->degree = 1;
    return type;
}

struct TypeNode *compile_array(struct Node *node, struct Array *this, struct CPContext *context) {
    error_semantic("Array is not supported currently", node);
    int size = vsize(&this->values);
    if (size == 0) {
        error_semantic("Array has to be non-empty", node);
    }
    int idx = context->bss_index;
    context->bss_index++;
    struct TypeNode *_type = NULL;
    codegen_buffer_zero(_concat("_B", _itoa(idx)), size * 8, context);
    for (int i = 0; i < size; i++) {
        struct TypeNode *_type2 = compile_node(this->values.ptr[i], context);
        if (i == 0) {
            _type = _type2;
        }
        if (!type_equal(_type, _type2, context)) {
            error_semantic("Array elements have to have same type", node);
        }
        codegen_move_labelreg(REG_A, _concat("_B", _itoa(idx)), context);
        codegen_stack_popword(REG_B, context);
        _fputsi(context->fd_text, "mov [rax + ", i * 8, "], rbx\n");
    }
    codegen_stack_pushlabel(_concat("_B", _itoa(idx)), context);
    _type = type_copy_node(_type);
    _type->degree++;
    return _type;
}

struct TypeNode *compile_struct_instance(struct Node *node, struct StructInstance *this, struct CPContext *context) {
    struct TypeNode *type_node = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    type_node->degree = 0;
    
    struct TypeStruct *type = (struct TypeStruct*)_malloc(sizeof(struct TypeStruct));
    type_node->node_type = TypeNodeStruct;
    type_node->node_ptr = type;
    type->names = vnew();
    type->types = vnew();

    int old_sf_pos = context->sf_pos;

    int sz = vsize(&this->names);
    int orig_struct_size = 0;
    int packed_struct_size = 0;
    for (int i = sz - 1; i >= 0; i--) {
        struct TypeNode *_type = compile_node(this->values.ptr[i], context);
        int field_size = type_size(_type, context);
        orig_struct_size += align_to_word(field_size);
        packed_struct_size += field_size;
        context->sf_pos -= align_to_word(field_size);
        codegen_stack_shift(-align_to_word(field_size), context);
        vpush(&type->names, this->names.ptr[i]);
        vpush(&type->types, _type);
    }

    vreverse(&type->names);
    vreverse(&type->types);

    codegen_stack_shift(orig_struct_size, context);

    int ptr1 = 0;
    int ptr2 = 0;
    for (int i = 0; i < sz; i++) {
        const char *dst = _concat3("[rsp - ", _itoa(orig_struct_size + packed_struct_size - ptr1), "]");
        const char *src = _concat3("[rsp - ", _itoa(orig_struct_size - ptr2), "]");
        int tsize = type_size(type->types.ptr[i], context);
        compile_memcpy(dst, src, tsize, context);
        ptr1 += tsize;
        ptr2 += align_to_word(tsize);
    }

    const char *dst = _concat3("[rsp - ", _itoa(align_to_word(packed_struct_size)), "]");
    const char *src = _concat3("[rsp - ", _itoa(orig_struct_size + packed_struct_size), "]");
    compile_memcpy(dst, src, packed_struct_size, context);

    context->sf_pos = old_sf_pos;
    return type_node;
}

struct TypeNode *compile_lambda_function(struct Node *node, struct LambdaFunction *this, struct CPContext *context) {
    char *identifier_back = _concat("_Z", _itoa(context->function_index));
    context->function_index++;
    char *identifier_end = _concat("_E", identifier_back);

    struct TypeNode *type = from_signature_to_type(this->signature, context);

    struct TypeNode *_type = compile_function_signature(node, this->signature, context, this->block, identifier_back, identifier_end);    
    if (!type_equal(_type, this->signature->return_type, context)) {
        error_semantic("Function return type does not equal to the type of the function body", node);
    }
    
    codegen_stack_pushlabel(identifier_back, context);

    return type;
}

struct TypeNode *compile_sizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    if (!type_check(this->type, context)) {
        error_semantic("Type identifier was not declared in sizeof type", node);
    }

    int size = type_size(this->type, context);
    codegen_stack_pushlabel(_itoa(size), context);
    return context->node_int;
}

void compile_call_finish(struct CPContext *context, int sz) {
    codegen_stack_shift(8 * (sz + 1), context);
    context->sf_pos += 8 * (sz + 1);

    codegen_call_arguments_restore(sz, context);

    codegen_stack_popword(REG_A, context);
    codegen_call_reg(REG_A, context);
    codegen_stack_pushword(REG_A, context);
}

struct TypeNode *compile_function_call(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    struct TypeNode *type = compile_node(this->function, context);
    codegen_stack_shift(-8, context);
    context->sf_pos -= WORD;

    type = type_get_function(type, context);
    if (!type) {
        error_semantic("Function expected in function call", node);
    }
    struct TypeFunction *_type = type->node_ptr;

    if (_type->propagate_allocator) {
        vpush(&this->arguments, NULL);
        int sz = vsize(&this->arguments);
        for (int i = sz - 1; i >= 0; i--) {
            this->arguments.ptr[i] = this->arguments.ptr[i - 1];
        }
        if (this->propagate_allocator) {
            this->arguments.ptr[0] = this->propagate_allocator;
        }
        else if (context_find_variable(context, "@")) {
            struct Node *_node = (struct Node*)_malloc(sizeof(struct Node));
            _node->line_begin = 0;
            _node->position_begin = 0;
            _node->line_end = 0;
            _node->position_end = 0;
            _node->filename = "_generated";
            struct Identifier *identifier = (struct Identifier*)_malloc(sizeof(struct Identifier));
            _node->node_ptr = identifier;
            _node->node_type = NodeIdentifier;
            identifier->identifier = "@";
            identifier->address = false;
            this->arguments.ptr[0] = _node;
        }
        else {
            error_semantic("Allocator expected for propagation to called function", node);
        }
    }
    else {
        if (this->propagate_allocator) {
            error_semantic("Unexpected allocator propagation to called function", node);
        }
    }

    int sz = vsize(&_type->types);
    if (sz != vsize(&this->arguments)) {
        error_semantic("Incorrect number of arguments in function call", node);
    }
    
    for (int i = 0; i < sz; i++) {
        struct TypeNode *type_arg = compile_node(this->arguments.ptr[i], context);
        if (!type_equal(type_arg, _type->types.ptr[i], context)) {
            error_semantic("Passing to function value of incorrect type", node);
        }
        codegen_stack_shift(-8, context);
        context->sf_pos -= WORD;
    }

    compile_call_finish(context, sz);
    return _type->return_type;
}

struct TypeNode *compile_method_call(struct Node *node, struct MethodCall *this, struct CPContext *context) {
    codegen_stack_shift(-8, context);
    context->sf_pos -= 8;
    struct TypeNode *type_caller = compile_node(this->caller, context);
    codegen_stack_shift(-8, context);
    context->sf_pos -= 8;

    struct TypeNode *type_function;
    struct FunctionInfo *function_info = context_find_method(context, this->function, type_caller);
    if (function_info) {
        type_function = function_info->type;
        codegen_move_labelreg(REG_A, function_info->name_back, context);
        codegen_stack_pushword_phase(REG_A, 8, context);
    }
    else {
        error_semantic("Method was not declared", node);
    }

    struct TypeFunction *_type = type_function->node_ptr;
    int sz = vsize(&_type->types);
    if (sz != vsize(&this->arguments) + 1) {
        error_semantic("Incorrect number of arguments in function call", node);
    }

    for (int i = 1; i < sz; i++) {
        struct TypeNode *type_arg = compile_node(this->arguments.ptr[i - 1], context);
        if (!type_equal(type_arg, _type->types.ptr[i], context)) {
            error_semantic("Passing to function value of incorrect type", node);
        }
        codegen_stack_shift(-8, context);
        context->sf_pos -= WORD;
    }

    compile_call_finish(context, sz);
    return _type->return_type;
}

struct TypeNode *compile_dereference(struct Node *node, struct Dereference *this, struct CPContext *context) {
    struct TypeNode *_type = compile_node(this->expression, context);
    if (_type->degree == 0) error_semantic("Dereference of not pointer value", node);
    _type = type_copy_node(_type);
    _type->degree--;
    int _type_size = type_size(_type, context);

    codegen_stack_popword(REG_A, context);
    const char *dst = _concat3("[rsp - ", _itoa(align_to_word(_type_size)), "]");
    const char *src = "[rax]";
    compile_memcpy(dst, src, align_to_word(_type_size), context);

    return _type;
}

struct TypeNode *compile_index(struct Node *node, struct Index *this, struct CPContext *context) {
    struct TypeNode *_type1 = compile_node(this->left, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in indexation", node);

    codegen_stack_shift(-8, context);
    context->sf_pos -= 8;

    struct TypeNode *_type2 = compile_node(this->right, context);
    if (!type_equal(_type2, context->node_int, context)) {
        error_semantic("Integer expected in indexation", node);
    }
    
    codegen_stack_shift(8, context);
    context->sf_pos += 8;
    codegen_stack_popword_phase(REG_A, -16, context);
    _type1->degree--;
    int _type_size = type_size(_type1, context);
    _fputsi(context->fd_text, "mov rbx, ", _type_size, "\n");
    _type1->degree++;
    _fputs(context->fd_text, "mul rbx\n");
    _fputs(context->fd_text, "add rax, [rsp - 8]\n");
    if (!this->address) {
        // const char *dst = _concat3("[rsp - ", _itoa(align_to_word(_type_size)), "]");
        // const char *src = "[rax]";
        // compile_memcpy(dst, src, align_to_word(_type_size), context);
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
        codegen_stack_pushword(REG_B, context);
        _type1 = type_copy_node(_type1);
        _type1->degree--;
    }
    else {
        codegen_stack_pushword(REG_A, context);
    }
    return _type1;
}

struct TypeNode *compile_get_field(struct Node *node, struct GetField *this, struct CPContext *context) {
    struct TypeNode *type = compile_node(this->left, context);
    type = type_get_struct_pointer(type, context);
    if (!type) {
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
    
    codegen_stack_popword(REG_A, context);
    if (!this->address) {    
        const char *dst = _concat3("[rsp - ", _itoa(align_to_word(type_size(field_type, context))), "]");
        const char *src = _concat3("[rax + ", _itoa(phase), "]");
        compile_memzero(dst, align_to_word(type_size(field_type, context)), context);
        compile_memcpy(dst, src, type_size(field_type, context), context);
    }
    else {
        _fputsi(context->fd_text, "lea rbx, [rax + ", phase, "]\n");
        codegen_stack_pushword(REG_B, context);
        field_type = type_copy_node(field_type);
        field_type->degree++;
    }

    return field_type;
}

struct TypeNode *compile_arithmetic(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {    
    struct TypeNode *_type1 = NULL;
    if (this->left) {
        _type1 = compile_node(this->left, context);
        if (!type_equal(_type1, context->node_int, context) && 
            !type_equal(_type1, context->node_char, context)) {
            error_semantic("Integer type expected in arithmetic operation", this->left);
        }
    }

    codegen_stack_shift(-8, context);
    context->sf_pos -= 8;

    struct TypeNode *_type2 = compile_node(this->right, context);
    if (!type_equal(_type2, context->node_int, context) && 
        !type_equal(_type2, context->node_char, context)) {
        error_semantic("Integer type expected in arithmetic operator", this->right);
    }

    if (_type1 && !type_equal(_type1, _type2, context)) {
        error_semantic("Equal types expected in arithmetic operator", node);
    }

    codegen_stack_shift(8, context);
    context->sf_pos += 8;

    return _type2;
}

struct TypeNode *compile_and(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rax, rax\n");
    _fputs(context->fd_text, "sub qword [rsp - 8], 0\n");
    _fputs(context->fd_text, "setne al\n");
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
    _fputs(context->fd_text, "setne bl\n");
    _fputs(context->fd_text, "and rax, rbx\n");
    codegen_stack_pushword(REG_A, context);
    return context->node_int;
}

struct TypeNode *compile_or(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rax, rax\n");
    _fputs(context->fd_text, "sub qword [rsp - 8], 0\n");
    _fputs(context->fd_text, "setne al\n");
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
    _fputs(context->fd_text, "setne bl\n");
    _fputs(context->fd_text, "or rax, rbx\n");
    codegen_stack_pushword(REG_A, context);
    return context->node_int;
}

struct TypeNode *compile_not(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "sub qword [rsp - 16], 0\n");
    _fputs(context->fd_text, "sete bl\n");
    codegen_stack_pushword(REG_B, context);
    return context->node_int;
}

struct TypeNode *compile_bitwise_and(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "and rax, [rsp - 16]\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_bitwise_or(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "or rax, [rsp - 16]\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_bitwise_xor(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "xor rax, [rsp - 16]\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_bitwise_not(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputs(context->fd_text, "not rax\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_bitwise_shift_left(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
    _fputs(context->fd_text, "shl rax, cl\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_bitwise_shift_right(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
    _fputs(context->fd_text, "shr rax, cl\n");
    codegen_stack_pushword(REG_A, context);
    return _type;
}

struct TypeNode *compile_addition(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "add rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_subtraction(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    if (this->left) _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    else _fputs(context->fd_text, "mov rax, 0\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_multiplication(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, [rsp - 16]\n");
    _fputs(context->fd_text, "mul rdx\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_division(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, 0\n");
    _fputs(context->fd_text, "div qword [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_modulo(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rdx, 0\n");
    _fputs(context->fd_text, "div qword [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rdx\n");
    return _type;
}

struct TypeNode *compile_less(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "setl bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_greater(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "setg bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "sete bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_less_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "setle bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_greater_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "setge bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_not_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "xor rbx, rbx\n");
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    _fputs(context->fd_text, "setne bl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
    return context->node_int;
}

struct TypeNode *compile_node(struct Node *node, struct CPContext *context) {
    _fputs(context->fd_text, "; ");
    _fputs(context->fd_text, node->filename);
    _fputs(context->fd_text, " ");
    _fputi(context->fd_text, node->line_begin + 1);
    _fputs(context->fd_text, ":");
    _fputi(context->fd_text, node->position_begin + 1);
    _fputs(context->fd_text, " -> ");

    if (node->node_type == NodeModule) {
        _fputs(context->fd_text, "module\n");
        compile_module(node, (struct Module*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBlock) {
        _fputs(context->fd_text, "block\n");
        return compile_block(node, (struct Block*)node->node_ptr, context);
    }
    else if (node->node_type == NodeInclude) {
        _fputs(context->fd_text, "include\n");
        compile_include(node, (struct Include*)node->node_ptr, context);
    }
    else if (node->node_type == NodeTest) {
        _fputs(context->fd_text, "test\n");
        compile_test(node, (struct Test*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIf) {
        _fputs(context->fd_text, "if\n");
        return compile_if(node, (struct If*)node->node_ptr, context);
    }
    else if (node->node_type == NodeWhile) {
        _fputs(context->fd_text, "while\n");
        return compile_while(node, (struct While*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionDefinition) {
        _fputs(context->fd_text, "function definition\n");
        compile_function_definition(node, (struct FunctionDefinition*)node->node_ptr, context);
    }
    else if (node->node_type == NodePrototype) {
        _fputs(context->fd_text, "prototype\n");
        compile_prototype(node, (struct Prototype*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGlobalDefinition) {
        _fputs(context->fd_text, "global definition\n");
        compile_global_definition(node, (struct GlobalDefinition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDefinition) {
        _fputs(context->fd_text, "definition\n");
        compile_definition(node, (struct Definition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeTypeDefinition) {
        _fputs(context->fd_text, "type definition\n");
        compile_type_definition(node, (struct TypeDefinition*)node->node_ptr, context);
    }
    else if (node->node_type == NodeReturn) {
        _fputs(context->fd_text, "return\n");
        compile_return(node, (struct Return*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBreak) {
        _fputs(context->fd_text, "break\n");
        compile_break(node, (struct Break*)node->node_ptr, context);
    }
    else if (node->node_type == NodeContinue) {
        _fputs(context->fd_text, "continue\n");
        compile_continue(node, (struct Continue*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAs) {
        _fputs(context->fd_text, "as\n");
        return compile_as(node, (struct As*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAssignment) {
        _fputs(context->fd_text, "assignment\n");
        compile_assignment(node, (struct Assignment*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMovement) {
        _fputs(context->fd_text, "movement\n");
        compile_movement(node, (struct Movement*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIdentifier) {
        _fputs(context->fd_text, "identifier\n");
        return compile_identifier(node, (struct Identifier*)node->node_ptr, context);
    }
    else if (node->node_type == NodeInteger) {
        _fputs(context->fd_text, "integer\n");
        return compile_integer(node, (struct Integer*)node->node_ptr, context);
    }
    else if (node->node_type == NodeChar) {
        _fputs(context->fd_text, "char\n");
        return compile_char(node, (struct Char*)node->node_ptr, context);
    }
    else if (node->node_type == NodeString) {
        _fputs(context->fd_text, "string\n");
        return compile_string(node, (struct String*)node->node_ptr, context);
    }
    else if (node->node_type == NodeArray) {
        _fputs(context->fd_text, "array\n");
        return compile_array(node, (struct Array*)node->node_ptr, context);
    }
    else if (node->node_type == NodeStructInstance) {
        _fputs(context->fd_text, "struct instance\n");
        return compile_struct_instance(node, (struct StructInstance*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLambdaFunction) {
        _fputs(context->fd_text, "lambda function\n");
        return compile_lambda_function(node, (struct LambdaFunction*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSizeof) {
        _fputs(context->fd_text, "sizeof\n");
        return compile_sizeof(node, (struct Sizeof*)node->node_ptr, context);
    }
    else if (node->node_type == NodeFunctionCall) {
        _fputs(context->fd_text, "function call\n");
        return compile_function_call(node, (struct FunctionCall*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMethodCall) {
        _fputs(context->fd_text, "method call\n");
        return compile_method_call(node, (struct MethodCall*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDereference) {
        _fputs(context->fd_text, "dereference\n");
        return compile_dereference(node, (struct Dereference*)node->node_ptr, context);
    }
    else if (node->node_type == NodeIndex) {
        _fputs(context->fd_text, "index\n");
        return compile_index(node, (struct Index*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGetField) {
        _fputs(context->fd_text, "get field\n");
        return compile_get_field(node, (struct GetField*)node->node_ptr, context);
    }

    else if (node->node_type == NodeAnd) {
        _fputs(context->fd_text, "and\n");
        return compile_and(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeOr) {
        _fputs(context->fd_text, "or\n");
        return compile_or(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeNot) {
        _fputs(context->fd_text, "not\n");
        return compile_not(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseAnd) {
        _fputs(context->fd_text, "bitwise and\n");
        return compile_bitwise_and(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseOr) {
        _fputs(context->fd_text, "bitwise or\n");
        return compile_bitwise_or(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseXor) {
        _fputs(context->fd_text, "bitwise xor\n");
        return compile_bitwise_xor(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseNot) {
        _fputs(context->fd_text, "bitwise not\n");
        return compile_bitwise_not(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseShiftLeft) {
        _fputs(context->fd_text, "bitwise shift left\n");
        return compile_bitwise_shift_left(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseShiftRight) {
        _fputs(context->fd_text, "bitwise shift right\n");
        return compile_bitwise_shift_right(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeAddition) {
        _fputs(context->fd_text, "addition\n");
        return compile_addition(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeSubtraction) {
        _fputs(context->fd_text, "subtraction\n");
        return compile_subtraction(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeMultiplication) {
        _fputs(context->fd_text, "multiplication\n");
        return compile_multiplication(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeDivision) {
        _fputs(context->fd_text, "division\n");
        return compile_division(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeModulo) {
        _fputs(context->fd_text, "modulo\n");
        return compile_modulo(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLess) {
        _fputs(context->fd_text, "less\n");
        return compile_less(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGreater) {
        _fputs(context->fd_text, "greater\n");
        return compile_greater(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeEqual) {
        _fputs(context->fd_text, "equal\n");
        return compile_equal(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeLessEqual) {
        _fputs(context->fd_text, "less or equal\n");
        return compile_less_equal(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeGreaterEqual) {
        _fputs(context->fd_text, "greater or equal\n");
        return compile_greater_equal(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeNotEqual) {
        _fputs(context->fd_text, "not equal\n");
        return compile_not_equal(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    return NULL;
}
