#include <ir_compile_c.h>
#include <stdio.h>
#include <string.h>
#include <posix.h>

void ir_print_type(struct TypeNode *type, int fd_text, bool in_parenthesis);

void ir_print_type_prefix(struct TypeNode *type, int fd_text) {
    switch (type->node_type) {
        case TypeNodeVoid: {
            _fputs(fd_text, "void");
            for (int i = 0; i < type->degree; i++) {
                _fputs(fd_text, "*");
            }
            break;
        }
        case TypeNodeChar: {
            _fputs(fd_text, "char");
            for (int i = 0; i < type->degree; i++) {
                _fputs(fd_text, "*");
            }
            break;
        }
        case TypeNodeInt: {
            _fputs(fd_text, "long");
            for (int i = 0; i < type->degree; i++) {
                _fputs(fd_text, "*");
            }
            break;
        }
        case TypeNodeFunction: {
            struct TypeFunction *type_function = (struct TypeFunction*)type->node_ptr;
            ir_print_type(type_function->return_type, fd_text, false);
            _fputs(fd_text, "(");
            for (int i = 0; i < type->degree + 1; i++) {
                _fputs(fd_text, "*");
            }
            break;
        }
        case TypeNodeArray:
        case TypeNodeStruct: {
            switch (type->size) {
                case -1: _panic("Unexpected type"); break;
                case 1: _fputs(fd_text, "char"); break;
                case 8: _fputs(fd_text, "long"); break;
                default: {
                    _fputs(fd_text, "char(");
                    for (int i = 0; i < type->degree; i++) {
                        _fputs(fd_text, "*");
                    }
                }
            }
            break;
        }
        default: {
            _panic("Unexpected type");
        }
    }
    _fputs(fd_text, " ");
}

void ir_print_type_suffix(struct TypeNode *type, int fd_text) {
    switch (type->node_type) {
        case TypeNodeVoid:
        case TypeNodeChar:
        case TypeNodeInt: {
            break;
        }
        case TypeNodeFunction: {
            struct TypeFunction *type_function = (struct TypeFunction*)type->node_ptr;
            _fputs(fd_text, ")(");
            int cnt_args = vsize(&type_function->types);
            for (int i = 0; i < cnt_args; i++) {
                struct TypeNode *arg = type_function->types.ptr[i];
                ir_print_type(arg, fd_text, false);
                if (i + 1 < cnt_args) {
                    _fputs(fd_text, ", ");
                }
            }
            _fputs(fd_text, ")");
            break;
        }
        case TypeNodeArray:
        case TypeNodeStruct: {
            switch (type->size) {
                case -1: _panic("Unexpected type"); break;
                case 1: ; break;
                case 8: ; break;
                default: _fputsi(fd_text, ")[", type->size, "]");
            }
            break;
        }
        default: {
            _panic("Unexpected type");
        }
    }
}

void ir_print_type(struct TypeNode *type, int fd_text, bool in_parenthesis) {
    if (in_parenthesis) _fputs(fd_text, "(");
    ir_print_type_prefix(type, fd_text);
    ir_print_type_suffix(type, fd_text);
    if (in_parenthesis) _fputs(fd_text, ")");
}

void ir_print_value(struct IRNode *value, int fd_text) {
    if (value->node_type == IRNodeConst) {
        struct IRConst *_value = value->node_ptr;
        _fputi(fd_text, _value->value);
    }
    else if (value->node_type == IRNodeGlobal) {
        struct IRGlobal *_value = value->node_ptr;
        if (!(value->type->node_type == TypeNodeChar && value->type->degree == 1) &&
            !(value->type->node_type == TypeNodeFunction && value->type->degree == 0)) {
            _fputs(fd_text, "&");
        }
        _fputs(fd_text, _value->name);
    }
    else {
        _fputsi(fd_text, "_v", value->idx, "");
    }
}

void ir_print_block(struct IRBlock *block, int fd_text) {
    _fputsi(fd_text, "_b", block->idx, "");
}

void ir_compile_phi(struct IRBlock *block, struct IRBlock *succ_block, int fd_text) {
    int sz_insts = vsize(&succ_block->value_list);
    for (int i = 0; i < sz_insts; i++) {
        struct IRNode *value = succ_block->value_list.ptr[i];
        if (value->node_type == IRNodePhi) {
            struct IRPhi *phi = value->node_ptr;
            int sz_args = vsize(&phi->values);
            for (int j = 0; j < sz_args; j++) {
                if (block == phi->blocks.ptr[j] && value != phi->values.ptr[j]) {
                    _fputs(fd_text, "    ");
                    ir_print_value(value, fd_text);
                    _fputs(fd_text, " = ");
                    ir_print_value(phi->values.ptr[j], fd_text);
                    _fputs(fd_text, ";\n");
                }
            }
        }
    }
}

void ir_compile_function_signature(struct IRFunction *function, int fd_text) {
    struct TypeFunction *type_function = (struct TypeFunction*)(function->type->node_ptr);
    ir_print_type_prefix(type_function->return_type, fd_text);
    _fputs(fd_text, function->name_generate);
    ir_print_type_suffix(type_function->return_type, fd_text);
    _fputs(fd_text, "(");
    int sz_args = vsize(&type_function->types);
    for (int j = 0; j < sz_args; j++) {
        ir_print_type_prefix(type_function->types.ptr[j], fd_text);
        ir_print_value(function->arg_list.ptr[j], fd_text);
        ir_print_type_suffix(type_function->types.ptr[j], fd_text);
        if (j + 1 != sz_args) {
            _fputs(fd_text, ", ");
        }
    }
    _fputs(fd_text, ")");
}

void ir_compile_value(struct IRBuilder *builder, struct IRBlock *block, struct IRNode *value, int fd_text) {
    enum IRNodeType type = value->node_type;

    if (type == IRNodeAlloca) return;

    if (type == IRNodePhi) {
        _fputs(fd_text, "// ");
    }
    
    if (ir_value_has_value(value)) {
        _fputs(fd_text, "    ");
        ir_print_value(value, fd_text);
        _fputs(fd_text, " = ");
    }

    if (type == IRNodePhi) {
        struct IRPhi *phi = value->node_ptr;
        int sz_args = vsize(&phi->values);
        _fputs(fd_text, "phi ");
        for (int arg = 0; arg < sz_args; arg++) {
            _fputs(fd_text, "[");
            ir_print_value(phi->values.ptr[arg], fd_text);
            _fputs(fd_text, ", ");
            ir_print_block(phi->blocks.ptr[arg], fd_text);
            _fputs(fd_text, "]");
            if (arg + 1 != sz_args) {
                _fputs(fd_text, ", ");
            }
        }
        _fputs(fd_text, ";");
    }
    else if (type == IRNodeGEP) {
        struct IRGEP *gep = value->node_ptr;
        ir_print_type(value->type, fd_text, true);
        _fputs(fd_text, "((void*)");
        ir_print_value(gep->base, fd_text);
        _fputs(fd_text, " + ");
        ir_print_value(gep->index, fd_text);
        _fputsi(fd_text, " * ", gep->size, ");");
    }
    else if (type == IRNodeSGEP) {
        struct IRSGEP *sgep = value->node_ptr;
        ir_print_type(value->type, fd_text, true);
        _fputs(fd_text, "((void*)");
        ir_print_value(sgep->instance, fd_text);
        _fputsi(fd_text, " + ", sgep->phase, ");");
    }
    else if (type == IRNodeLoad) {
        struct IRLoad *load = value->node_ptr;
        int sz = load->size;
        _fputs(fd_text, "*");
        ir_print_value(load->src, fd_text);
        _fputs(fd_text, ";");
    }
    else if (type == IRNodeStore) {
        struct IRStore *store = value->node_ptr;
        int sz = store->size;
        if (sz > 8) {
            for (int i = 0; i < sz; i++) {
                _fputs(fd_text, "    ((char*)");
                ir_print_value(store->dst, fd_text);
                _fputsi(fd_text, ")[", i, "] = (char)(*");
                ir_print_value(store->src, fd_text);
                _fputsi(fd_text, ")[", i, "]; // store");
                if (i + 1 < sz) _fputs(fd_text, "\n");
            }
        }
        else {
            _fputs(fd_text, "    *");
            ir_print_value(store->dst, fd_text);
            _fputs(fd_text, " = ");
            ir_print_value(store->src, fd_text);
            _fputs(fd_text, ";");
        }
    }
    else if (type == IRNodeCall) {
        struct IRCall *call = value->node_ptr;
        if (value->type->size == 0) {
            _fputs(fd_text, "    ");
        }
        ir_print_type(value->type, fd_text, true);
        int sz_args = vsize(&call->arguments);
        ir_print_value(call->function, fd_text);
        struct TypeFunction *type_function = (struct TypeFunction*)call->function->type->node_ptr;
        _fputs(fd_text, "(");
        for (int arg = 0; arg < sz_args; arg++) {
            ir_print_type((struct TypeNode*)type_function->types.ptr[arg], fd_text, true);
            struct IRNode *value_arg = call->arguments.ptr[arg];
            ir_print_value(value_arg, fd_text);
            if (arg + 1 != sz_args) {
                _fputs(fd_text, ", ");
            }
        }
        _fputs(fd_text, ");");
    }
    else if (type == IRNodeBr) {
        struct IRBr *br = value->node_ptr;
        ir_compile_phi(block, br->block, fd_text);
        _fputs(fd_text, "    goto ");
        ir_print_block(br->block, fd_text);
        _fputs(fd_text, ";\n");
    }
    else if (type == IRNodeCondBr) {
        struct IRCondBr *condbr = value->node_ptr;
        _fputs(fd_text, "    if (");
        ir_print_value(condbr->condition, fd_text);
        _fputs(fd_text, ") {\n");
        ir_compile_phi(block, condbr->block_then, fd_text);
        _fputs(fd_text, "    goto ");
        ir_print_block(condbr->block_then, fd_text);
        _fputs(fd_text, ";\n    } else {\n");
        ir_compile_phi(block, condbr->block_else, fd_text);
        _fputs(fd_text, "    goto ");
        ir_print_block(condbr->block_else, fd_text);
        _fputs(fd_text, ";\n    }\n");
    }
    else if (type == IRNodeRet) {
        struct IRRet *ret = value->node_ptr;
        _fputs(fd_text, "    return");
        if (ret->value) {
            _fputs(fd_text, " ");
            ir_print_value(ret->value, fd_text);
        }
        _fputs(fd_text, ";\n");
    }
    else if (type == IRNodeAnd ||
        type == IRNodeOr ||
        type == IRNodeNot ||
        type == IRNodeBitwiseAnd ||
        type == IRNodeBitwiseOr ||
        type == IRNodeBitwiseXor ||
        type == IRNodeBitwiseNot ||
        type == IRNodeBitwiseShiftLeft ||
        type == IRNodeBitwiseShiftRight ||
        type == IRNodeAddition ||
        type == IRNodeSubtraction ||
        type == IRNodeMultiplication ||
        type == IRNodeDivision ||
        type == IRNodeModulo ||
        type == IRNodeLess ||
        type == IRNodeGreater ||
        type == IRNodeEqual ||
        type == IRNodeLessEqual ||
        type == IRNodeGreaterEqual ||
        type == IRNodeNotEqual
    ) {
        struct IRBinaryOperator *binary_operator = value->node_ptr;
        if (binary_operator->left) {
            ir_print_value(binary_operator->left, fd_text);
            _fputs(fd_text, " ");
        }
        switch (type) {
            case IRNodeAnd:                 _fputs(fd_text, "&&"); break;
            case IRNodeOr:                  _fputs(fd_text, "||"); break;
            case IRNodeNot:                 _fputs(fd_text, "!"); break;
            case IRNodeBitwiseAnd:          _fputs(fd_text, "&"); break;
            case IRNodeBitwiseOr:           _fputs(fd_text, "|"); break;
            case IRNodeBitwiseXor:          _fputs(fd_text, "^"); break;
            case IRNodeBitwiseNot:          _fputs(fd_text, "~"); break;
            case IRNodeBitwiseShiftLeft:    _fputs(fd_text, "<<"); break;
            case IRNodeBitwiseShiftRight:   _fputs(fd_text, ">>"); break;
            case IRNodeAddition:            _fputs(fd_text, "+"); break;
            case IRNodeSubtraction:         _fputs(fd_text, "-"); break;
            case IRNodeMultiplication:      _fputs(fd_text, "*"); break;
            case IRNodeDivision:            _fputs(fd_text, "/"); break;
            case IRNodeModulo:              _fputs(fd_text, "%"); break;
            case IRNodeLess:                _fputs(fd_text, "<"); break;
            case IRNodeGreater:             _fputs(fd_text, ">"); break;
            case IRNodeEqual:               _fputs(fd_text, "=="); break;
            case IRNodeLessEqual:           _fputs(fd_text, "<="); break;
            case IRNodeGreaterEqual:        _fputs(fd_text, ">="); break;
            case IRNodeNotEqual:            _fputs(fd_text, "!="); break;
        }
        _fputs(fd_text, " ");
        ir_print_value(binary_operator->right, fd_text);
        _fputs(fd_text, ";");
    }
    else {
        _panic("Uncompilable value");
    }

    _fputs(fd_text, "\n");
}

void ir_compile_function(struct IRBuilder *builder, struct IRFunction *function, int fd_text) {
    int sz_blocks = vsize(&function->block_list);
    if (!sz_blocks) return;
    int sz_args = vsize(&function->arg_list);

    ir_compile_function_signature(function, fd_text);
    _fputs(fd_text, " {\n");

    for (int j = 0; j < sz_blocks; j++) {
        struct IRBlock *block = function->block_list.ptr[j];

        int sz_insts = vsize(&block->value_list);
        for (int k = 0; k < sz_insts; k++) {
            struct IRNode *value = block->value_list.ptr[k];
            enum IRNodeType type = value->node_type;

            if (type != IRNodeAlloca && ir_value_has_value(value)) {
                _fputs(fd_text, "    ");
                ir_print_type_prefix(value->type, fd_text);
                ir_print_value(value, fd_text);
                ir_print_type_suffix(value->type, fd_text);
                _fputsi(fd_text, "; // ", type, "\n");
            }
            if (type == IRNodeAlloca) {
                int sz = ((struct IRAlloca*)value->node_ptr)->size;
                _fputs(fd_text, "    char _");
                ir_print_value(value, fd_text);
                _fputsi(fd_text, "[", sz, "];\n");
                _fputs(fd_text, "    ");
                ir_print_type_prefix(value->type, fd_text);
                ir_print_value(value, fd_text);
                ir_print_type_suffix(value->type, fd_text);
                _fputs(fd_text, " = ");
                ir_print_type(value->type, fd_text, true);
                _fputs(fd_text, "&_");
                ir_print_value(value, fd_text);
                _fputsi(fd_text, "; // ", type, "\n");
            }
        }
    }

    for (int j = 0; j < sz_blocks; j++) {
        struct IRBlock *block = function->block_list.ptr[j];
        ir_print_block(block, fd_text);
        _fputs(fd_text, ":\n");

        int sz_insts = vsize(&block->value_list);
        for (int k = 0; k < sz_insts; k++) {
            struct IRNode *value = block->value_list.ptr[k];
            ir_compile_value(builder, block, value, fd_text);
            if (ir_value_is_terminator(value)) break;
        }
    }

    _fputs(fd_text, "}\n\n");
}

void ir_compile(struct IRBuilder *builder, const char *filename_compile_output) {
    int fd[2];
    posix_pipe(fd);
    int fd_text = fd[1];
    int fd_text_out = fd[0];

    int sz_globalvar = vsize(&builder->globalvar_list);
    for (int i = 0; i < sz_globalvar; i++) {
        struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
        if (globalvar->type == IRGlobalVarFunction) continue;
        ir_print_type_prefix(globalvar->ir_value->type, fd_text);
        _fputs(fd_text, globalvar->name);
        ir_print_type_suffix(globalvar->ir_value->type, fd_text);
        _fputs(fd_text, " = ");
        if (globalvar->type == IRGlobalVarInt) {
            int value = (long)globalvar->value;
            _fputsi(fd_text, "", value, ";\n");
        }
        if (globalvar->type == IRGlobalVarString) {
            const char *value = globalvar->value;
            _fputs(fd_text, "\"");
            int len = _strlen(value);
            for (int j = 0; j < len; j++) {
                if (value[j] == '\n') _fputs(fd_text, "\\n");
                else _fputc(fd_text, value[j]);
            }
            _fputs(fd_text, "\";\n");
        }
    }
    _fputs(fd_text, "\n");

    int sz_functions = vsize(&builder->function_list);
    for (int i = 0; i < sz_functions; i++) {
        struct IRFunction *function = builder->function_list.ptr[i];        
        int sz_blocks = vsize(&function->block_list);
        int sz_args = vsize(&function->arg_list);

        ir_compile_function_signature(function, fd_text);
        _fputs(fd_text, ";\n\n");
    }

    for (int i = 0; i < sz_functions; i++) {
        struct IRFunction *function = builder->function_list.ptr[i];
        ir_compile_function(builder, function, fd_text);
    }

    if (builder->testing) {
        _fputs(fd_text, "void posix_exit(int);\n");
        _fputs(fd_text, "void test() {\n");
        int sz = vsize(&builder->test_names);

        if (sz) {
            _fputs(fd_text, "    if (");
            for (int i = 0; i < sz; i++) {
                _fputs2(fd_text, builder->test_names.ptr[i], "()");
                if (i + 1 < sz) _fputs(fd_text, " || ");
            }
            _fputs(fd_text, ") posix_exit(1);\n");
        }
        _fputs(fd_text, "    posix_exit(0);\n}\n");
    }

    posix_close(fd_text);
    char *str_text = read_file_descriptor(fd_text_out);
    posix_close(fd_text_out);
    if (filename_compile_output) {
        write_file(filename_compile_output, str_text);
    }
}
