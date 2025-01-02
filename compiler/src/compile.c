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

void compile_memcpy(const char *dst, const char *src, int sz, struct CPContext *context) {
    _fputs3(context->fd_text, "lea rdi, ", dst, "\n");
    _fputs3(context->fd_text, "lea rsi, ", src, "\n");
    _fputsi(context->fd_text, "mov rcx, ", sz, "\n");
    _fputs(context->fd_text, "rep movsb\n");
}

int align_to_word(int x) {
    return (x + WORD - 1) / WORD * WORD;
}

struct TypeNode *compile_node(struct Node *node, struct CPContext *context);

void generate_test_function(struct CPContext *context, struct Settings *settings) {
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

    compile_node(node, context);

    if (context->testing) {
        generate_test_function(context, settings);
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
    char *tmp = _concat(str_bss, str_data);
    _free(str_bss);
    _free(str_data);
    char *program = _concat(tmp, str_text);
    _free(tmp);
    _free(str_text);
    if (settings->filename_compile_output) {
        write_file(settings->filename_compile_output, program);
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
    _fputs2(context->fd_text, label_info->name_begin, ":\n");

    int old_cnt_var = vsize(&context->variables);
    int old_cnt_functions = vsize(&context->functions);
    
    for (int i = 0; i < vsize(&this->statement_list); i++) {
        compile_node(this->statement_list.ptr[i], context);
    }

    int cnt_var = vsize(&context->variables);
    int cnt_functions = vsize(&context->functions);
    _fputsi(context->fd_text, "add rsp, ", label_info->sf_pos - context->sf_pos, "\n");
    context->sf_pos = label_info->sf_pos;
    for (int i = 0; i < cnt_var - old_cnt_var; i++) {
        vpop(&context->variables);
    }
    for (int i = 0; i < cnt_functions - old_cnt_functions; i++) {
        vpop(&context->functions);
    }
    _fputs2(context->fd_text, label_info->name_end, ":\n");

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
    _fputs3(context->fd_text, "jmp ", identifier_end, "\n");
    _fputs2(context->fd_text, identifier_back, ":\n");
    _fputs(context->fd_text, "push rbp\n");
    _fputs(context->fd_text, "mov rbp, rsp\n");

    struct Vector variables_tmp = context->variables;
    context->variables = vnew();
    int sf_pos_tmp = context->sf_pos;
    context->sf_pos = 0;

    int sz = 0;
    if (this) {
        sz = vsize(&this->identifiers);
        for (int i = 0; i < sz; i++) {
            _fputs3(context->fd_text, "push ", regs[i], "\n");
            struct VariableInfo *var_info = (struct VariableInfo*)_malloc(sizeof(struct VariableInfo));
            var_info->name = this->identifiers.ptr[i];
            var_info->type = this->types.ptr[i];
            int size = align_to_word(type_size(var_info->type, context));
            var_info->sf_phase = context->sf_pos - size;
            context->sf_pos -= size;
            vpush(&context->variables, var_info);
        }
    }

    struct TypeNode *_type = compile_node(block, context);
    
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputsi(context->fd_text, "add rsp, ", -context->sf_pos, "\n");
    for (int i = 0; i < sz; i++) {
        _free(context->variables.ptr[i]);
    }
    vdrop(&context->variables);
    context->variables = variables_tmp;
    context->sf_pos = sf_pos_tmp;

    _fputs(context->fd_text, "leave\n");
    _fputs(context->fd_text, "ret\n");
    _fputs2(context->fd_text, identifier_end, ":\n");

    return _type;
}

void compile_test(struct Node *node, struct Test *this, struct CPContext *context) {
    vpush(&context->test_names, (void*)this->name);
    char *identifier_end = _concat("_T", this->name);

    if (context->header) {
        _fputs3(context->fd_text, "extern ", this->name, "\n");
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
        _fputsi(context->fd_text, "_L", idx + i, ":\n");
        compile_node(this->condition_list.ptr[i], context);
        _fputs(context->fd_text, "cmp qword [rsp - 8], 0\n");
        _fputsi(context->fd_text, "je _L", idx + i + 1, "\n");
        struct TypeNode *_type2 = compile_node(this->block_list.ptr[i], context);
        if (_type && !type_equal(_type, _type2, context)) {
            error_semantic("If branches types do not equal", node);
        }
        if (!_type) _type = _type2;
        _fputsi(context->fd_text, "jmp _L", last, "\n");
    }
    _fputsi(context->fd_text, "_L", idx + sz, ":\n");
    if (this->else_block) {
        struct TypeNode *_type2 = compile_node(this->else_block, context);
        if (_type && !type_equal(_type, _type2, context)) {
            error_semantic("If branches types do not equal", node);
        }
        _fputsi(context->fd_text, "_L", idx + sz + 1, ":\n");
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
    _fputsi(context->fd_text, "_L", idx, ":\n");

    _fputsi(context->fd_text, "_L", idx, ":\n");
    compile_node(this->condition, context);
    _fputs(context->fd_text, "cmp qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "je _L", idx + 1, "\n");
    compile_node(this->block, context);
    _fputsi(context->fd_text, "jmp _L", idx, "\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    struct TypeNode *type_else = NULL;
    if (this->else_block) {
        type_else = compile_node(this->else_block, context);
    }
    _fputsi(context->fd_text, "_L", idx + 2, ":\n");

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

void compile_function_definition(struct Node *node, struct FunctionDefinition *this, struct CPContext *context) { 
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
    const char *identifier;
    if (this->caller_type) {
        identifier = _concat(type_mangle(this->caller_type, context), this->name);
    }
    else{
        identifier = this->name;
    }
    _fputs3(context->fd_text, "extern ", identifier, "\n");

    struct FunctionInfo *function_info = (struct FunctionInfo*)_malloc(sizeof(struct FunctionInfo));
    function_info->name_front = identifier;
    function_info->name_back = identifier;
    function_info->type = from_signature_to_type(this->signature);
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

void compile_definition(struct Node *node, struct Definition *this, struct CPContext *context) {
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
    _fputsi(context->fd_text, "sub rsp, ", sz, "\n");
}

void compile_type_definition(struct Node *node, struct TypeDefinition *this, struct CPContext *context) {
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

    _fputsi(context->fd_text, "add rsp, ", label_info->sf_pos - context->sf_pos, "\n");
    _fputs3(context->fd_text, "jmp ", label_info->name_end, "\n");
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

    _fputsi(context->fd_text, "add rsp, ", label_info->sf_pos - context->sf_pos, "\n");
    _fputs3(context->fd_text, "jmp ", label_info->name_end, "\n");
}

void compile_continue(struct Node *node, struct Continue *this, struct CPContext *context) {
    struct LabelInfo *label_info = context_find_loop_label(context, this->label);
    if (!label_info) {
        error_semantic("Label was not declared", node);
    }
    _fputsi(context->fd_text, "add rsp, ", label_info->sf_pos - context->sf_pos, "\n");
    _fputs3(context->fd_text, "jmp ", label_info->name_begin, "\n");
}

struct TypeNode *compile_as(struct Node *node, struct As *this, struct CPContext *context) {
    struct TypeNode *_type = compile_node(this->expression, context);
    return this->type;
}

void compile_assignment(struct Node *node, struct Assignment *this, struct CPContext *context) {
    if (this->dst->node_type != NodeIdentifier) {
        error_semantic("Identifier expected in assignment", node);
    }
    struct Identifier *_identifier = (struct Identifier*)(this->dst->node_ptr);
    struct VariableInfo *var_info = context_find_variable(context, _identifier->identifier);
    if (!var_info) {
        error_semantic("Variable was not declared in assignment", node);
    }
    struct TypeNode *_type1 = var_info->type;
    struct TypeNode *_type2 = compile_node(this->src, context);
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Assignment of not equal types", node);
    }

    int tsize = type_size(_type1, context);
    const char *dst = _concat3("[rbp + ", _itoa(var_info->sf_phase), "]");
    const char *src = _concat3("[rsp - ", _itoa(align_to_word(tsize)), "]");
    compile_memcpy(dst, src, tsize, context);
}

void compile_movement(struct Node *node, struct Movement *this, struct CPContext *context) {
    struct TypeNode *_type1 = compile_node(this->dst, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in movement", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= WORD;

    struct TypeNode *_type2 = compile_node(this->src, context);
    _type2->degree++;
    if (!type_equal(_type1, _type2, context)) {
        error_semantic("Movement of inappropriate types", node);
    }
    _type2->degree--;

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += WORD;

    int tsize = align_to_word(type_size(_type2, context));
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
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
        _fputs3(context->fd_text, "mov rax, ", function_info->name_back, "\n");
        _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    }
    else {
        struct VariableInfo *var_info = context_find_variable(context, this->identifier);
        if (var_info) {
            if (this->address) {
                _type = type_copy_node(var_info->type);
                _type->degree++;
                _fputsi(context->fd_text, "lea rax, [rbp + ", var_info->sf_phase, "]\n");
                _fputs(context->fd_text, "mov [rsp - 8], rax\n");
            }
            else {
                _type = var_info->type;
                const char *dst = _concat3("[rsp - ", _itoa(align_to_word(type_size(_type, context))), "]");
                const char *src = _concat3("[rbp + ", _itoa(var_info->sf_phase), "]");
                compile_memcpy(dst, src, align_to_word(type_size(_type, context)), context);
            }
        }
        else {
            error_semantic("Variable was not declared", node);
        }
    }

    return _type;
}

struct TypeNode *compile_integer(struct Node *node, struct Integer *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return context->node_int;
}

struct TypeNode *compile_char(struct Node *node, struct Char *this, struct CPContext *context) {
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", this->value, "\n");
    return context->node_char;
}

struct TypeNode *compile_string(struct Node *node, struct String *this, struct CPContext *context) {
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

struct TypeNode *compile_array(struct Node *node, struct Array *this, struct CPContext *context) {
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
        struct TypeNode *_type2 = compile_node(this->values.ptr[i], context);
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
        _fputsi(context->fd_text, "sub rsp, ", align_to_word(field_size), "\n");
        vpush(&type->names, this->names.ptr[i]);
        vpush(&type->types, _type);
    }

    vreverse(&type->names);
    vreverse(&type->types);

    _fputsi(context->fd_text, "add rsp, ", orig_struct_size, "\n");

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

    struct TypeNode *type = from_signature_to_type(this->signature);

    struct TypeNode *_type = compile_function_signature(node, this->signature, context, this->block, identifier_back, identifier_end);    
    if (!type_equal(_type, this->signature->return_type, context)) {
        error_semantic("Function return type does not equal to the type of the function body", node);
    }
    
    _fputs3(context->fd_text, "mov qword [rsp - 8], ", identifier_back, "\n");

    return type;
}

struct TypeNode *compile_sizeof(struct Node *node, struct Sizeof *this, struct CPContext *context) {
    int size = type_size(this->type, context);
    _fputsi(context->fd_text, "mov qword [rsp - 8], ", size, "\n");
    struct TypeNode *type = (struct TypeNode*)_malloc(sizeof(struct TypeNode));
    struct TypeInt *_type = (struct TypeInt*)_malloc(sizeof(struct TypeInt));
    type->node_ptr = _type;
    type->node_type = TypeNodeInt;
    type->degree = 0;
    return type;
}

void compile_call_finish(struct CPContext *context, int sz) {
    _fputsi(context->fd_text, "add rsp, ", WORD * (sz + 1), "\n");
    context->sf_pos += WORD * (sz + 1);

    for (int i = 0; i < sz; i++) {
        _fputs3(context->fd_text, "mov ", regs[i], ", ");
        _fputsi(context->fd_text, "[rsp - ", WORD * (i + 2), "]\n");
    }

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "call rax\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
}

struct TypeNode *compile_function_call(struct Node *node, struct FunctionCall *this, struct CPContext *context) {
    struct TypeNode *type = compile_node(this->function, context);
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
        struct TypeNode *type_arg = compile_node(this->arguments.ptr[i], context);
        if (!type_equal(type_arg, _type->types.ptr[i], context)) {
            error_semantic("Passing to function value of incorrect type", node);
        }
        _fputs(context->fd_text, "sub rsp, 8\n");
        context->sf_pos -= WORD;
    }

    compile_call_finish(context, sz);
    return _type->return_type;
}

struct TypeNode *compile_method_call(struct Node *node, struct MethodCall *this, struct CPContext *context) {
    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= WORD;
    struct TypeNode *type_caller = compile_node(this->caller, context);
    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= WORD;

    struct TypeNode *type_function;
    struct FunctionInfo *function_info = context_find_method(context, this->function, type_caller);
    if (function_info) {
        type_function = function_info->type;
        _fputs3(context->fd_text, "mov rax, ", function_info->name_back, "\n");
        _fputs(context->fd_text, "mov [rsp + 8], rax\n");
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
        _fputs(context->fd_text, "sub rsp, 8\n");
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

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    const char *dst = _concat3("[rsp - ", _itoa(align_to_word(_type_size)), "]");
    const char *src = "[rax]";
    compile_memcpy(dst, src, align_to_word(_type_size), context);

    return _type;
}

struct TypeNode *compile_index(struct Node *node, struct Index *this, struct CPContext *context) {
    struct TypeNode *_type1 = compile_node(this->left, context);
    if (_type1->degree == 0) error_semantic("Pointer expected in indexation", node);

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct TypeNode *_type2 = compile_node(this->right, context);
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
    
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    if (!this->address) {    
        _fputsi(context->fd_text, "mov rbx, [rax + ", phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
        const char *dst = _concat3("[rsp - ", _itoa(align_to_word(type_size(field_type, context))), "]");
        const char *src = _concat3("[rax + ", _itoa(phase), "]");
        compile_memcpy(dst, src, align_to_word(type_size(field_type, context)), context);
    }
    else {
        _fputsi(context->fd_text, "lea rbx, [rax + ", phase, "]\n");
        _fputs(context->fd_text, "mov [rsp - 8], rbx\n");
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

    _fputs(context->fd_text, "sub rsp, 8\n");
    context->sf_pos -= 8;

    struct TypeNode *_type2 = compile_node(this->right, context);
    if (!type_equal(_type2, context->node_int, context) && 
        !type_equal(_type2, context->node_char, context)) {
        error_semantic("Integer type expected in arithmetic operator", this->right);
    }

    if (_type1 && !type_equal(_type1, _type2, context)) {
        error_semantic("Equal types expected in arithmetic operator", node);
    }

    _fputs(context->fd_text, "add rsp, 8\n");
    context->sf_pos += 8;

    return _type2;
}

struct TypeNode *compile_and(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    int idx = context->branch_index;
    context->branch_index += 2;

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, 0\n");
    _fputsi(context->fd_text, "je _L", idx, "\n");
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputs(context->fd_text, "sub rax, 0\n");
    _fputsi(context->fd_text, "je _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_or(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    int idx = context->branch_index;
    context->branch_index += 2;

    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, 0\n");
    _fputsi(context->fd_text, "jne _L", idx, "\n");
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputs(context->fd_text, "sub rax, 0\n");
    _fputsi(context->fd_text, "jne _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_not(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    int idx = context->branch_index;
    context->branch_index += 2;

    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputs(context->fd_text, "sub rax, 0\n");
    _fputsi(context->fd_text, "je _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_bitwise_and(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "and rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_bitwise_or(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "or rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_bitwise_xor(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "xor rax, [rsp - 16]\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_bitwise_not(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 16]\n");
    _fputs(context->fd_text, "not rax\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_bitwise_left(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
    _fputs(context->fd_text, "shl rax, cl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
    return _type;
}

struct TypeNode *compile_bitwise_right(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "mov rcx, [rsp - 16]\n");
    _fputs(context->fd_text, "shr rax, cl\n");
    _fputs(context->fd_text, "mov [rsp - 8], rax\n");
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
    return context->node_int;
}

struct TypeNode *compile_greater(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jg _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "je _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_less_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jle _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_greater_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jge _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
    return context->node_int;
}

struct TypeNode *compile_not_equal(struct Node *node, struct BinaryOperator *this, struct CPContext *context) {
    struct TypeNode *_type = compile_arithmetic(node, this, context);
    _fputs(context->fd_text, "mov rax, [rsp - 8]\n");
    _fputs(context->fd_text, "sub rax, [rsp - 16]\n");
    int idx = context->branch_index;
    context->branch_index += 2;
    _fputsi(context->fd_text, "jne _L", idx, "\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 0\n");
    _fputsi(context->fd_text, "jmp _L", idx + 1, "\n");
    _fputsi(context->fd_text, "_L", idx, ":\n");
    _fputs(context->fd_text, "mov qword [rsp - 8], 1\n");
    _fputsi(context->fd_text, "_L", idx + 1, ":\n");
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

    if (node->node_type == NodeBlock) {
        _fputs(context->fd_text, "block\n");
        return compile_block(node, (struct Block*)node->node_ptr, context);
    }
    if (node->node_type == NodeInclude) {
        _fputs(context->fd_text, "include\n");
        compile_include(node, (struct Include*)node->node_ptr, context);
    }
    if (node->node_type == NodeTest) {
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
        _fputs(context->fd_text, "addition\n");
        return compile_and(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeOr) {
        _fputs(context->fd_text, "addition\n");
        return compile_or(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeNot) {
        _fputs(context->fd_text, "addition\n");
        return compile_not(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseAnd) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_and(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseOr) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_or(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseXor) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_xor(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseNot) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_not(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseLeft) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_left(node, (struct BinaryOperator*)node->node_ptr, context);
    }
    else if (node->node_type == NodeBitwiseRight) {
        _fputs(context->fd_text, "addition\n");
        return compile_bitwise_right(node, (struct BinaryOperator*)node->node_ptr, context);
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
