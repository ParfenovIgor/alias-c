#pragma once

#include <vector.h>
#include <type.h>
#include <typeast.h>
#include <cassert.h>
#include <stdbool.h>
#include <panic.h>

enum IRValueType {
    IRNodeArg,
    IRNodeConst,
    IRNodeGlobal,
    IRNodePhi,
    IRNodeAddress,
    IRNodeGEP,
    IRNodeSGEP,
    IRNodeAlloca,
    IRNodeLoad,
    IRNodeStore,
    IRNodeCall,
    IRNodeBr,
    IRNodeCondBr,
    IRNodeRet,

    IRNodeAnd,
    IRNodeOr,
    IRNodeNot,
    IRNodeBitwiseAnd,
    IRNodeBitwiseOr,
    IRNodeBitwiseXor,
    IRNodeBitwiseNot,
    IRNodeBitwiseShiftLeft,
    IRNodeBitwiseShiftRight,
    IRNodeAddition,
    IRNodeSubtraction,
    IRNodeMultiplication,
    IRNodeDivision,
    IRNodeModulo,
    IRNodeLess,
    IRNodeGreater,
    IRNodeEqual,
    IRNodeLessEqual,
    IRNodeGreaterEqual,
    IRNodeNotEqual,
};

struct IRValue {
    enum IRValueType value_type;
    struct TypeNode *type;
    struct Vector value_arg_list;
    struct Vector block_arg_list;
    struct Vector const_arg_list;
    bool spill;
};

struct IRBlock {
    struct Vector value_list;
    struct Vector succ_list;
    struct Vector pred_list;

    struct Vector variable_name_list;
    struct Vector variable_value_list;
};

struct IRFunction {
    const char *name;
    struct TypeNode *type;
    struct Vector arg_list;
    struct Vector block_list;
    struct IRValue *ir_value;
};

enum IRGlobalVarType {
    IRGlobalVarInt,
    IRGlobalVarString,
    IRGlobalVarFunction,
};

struct IRGlobalVar {
    const char *name;
    enum IRGlobalVarType type;
    void *value;
    struct IRValue *ir_value;
};

struct IRBuilder {
    struct Vector function_list;
    struct Vector globalvar_list;
    struct IRFunction *current_function;
    struct IRBlock *current_block;

    bool header;

    struct Vector irblock_label_stack;
    struct Vector irblock_block_stack;
    struct Vector irblock_phi_stack;
    struct Vector irloop_label_stack;
    struct Vector irloop_blockcond_stack;
    struct Vector irloop_blockend_stack;
    struct Vector irloop_phi_stack;
};

struct IRBuilder *ir_builder() {
    struct IRBuilder *builder = (struct IRBuilder*)_malloc(sizeof(struct IRBuilder));
    builder->function_list = vnew();
    builder->globalvar_list = vnew();
    builder->irblock_label_stack = vnew();
    builder->irblock_block_stack = vnew();
    builder->irblock_phi_stack = vnew();
    builder->irloop_label_stack = vnew();
    builder->irloop_blockend_stack = vnew();
    builder->irloop_blockcond_stack = vnew();
    builder->irloop_phi_stack = vnew();
    builder->header = false;
    return builder;
}

struct IRValue *ir_build_value(struct IRBuilder *builder, enum IRValueType value_type) {
    struct IRValue *value = (struct IRValue*)_malloc(sizeof(struct IRValue));
    vpush(&builder->current_block->value_list, value);
    value->value_type = value_type;
    value->value_arg_list = vnew();
    value->block_arg_list = vnew();
    value->const_arg_list = vnew();
    value->spill = false;
    return value;
}

struct IRValue *ir_build_value_free(struct IRBuilder *builder, enum IRValueType value_type) {
    struct IRValue *value = (struct IRValue*)_malloc(sizeof(struct IRValue));
    value->value_type = value_type;
    value->value_arg_list = vnew();
    value->block_arg_list = vnew();
    value->const_arg_list = vnew();
    value->spill = false;
    return value;
}

struct IRBlock *ir_build_block(struct IRBuilder *builder) {
    struct IRBlock *block = (struct IRBlock*)_malloc(sizeof(struct IRBlock));
    vpush(&builder->current_function->block_list, block);
    block->value_list = vnew();
    block->succ_list = vnew();
    block->pred_list = vnew();
    block->variable_name_list = vnew();
    block->variable_value_list = vnew();
    return block;
}

struct IRFunction *ir_build_function(struct IRBuilder *builder) {
    struct IRFunction *function = (struct IRFunction*)_malloc(sizeof(struct IRFunction));
    vpush(&builder->function_list, function);
    function->arg_list = vnew();
    function->block_list = vnew();
    return function;
}

void ir_print_value(struct Vector *values_list, struct IRValue *value) {
    if (value->value_type == IRNodeConst) {
        _assert(vsize(&value->const_arg_list) == 2);
        int size = (long)value->const_arg_list.ptr[0];
        int val = (long)value->const_arg_list.ptr[1];
        switch (size) {
            case 0: _fputs(STDERR, "void"); break;
            case 1: _fputsi(STDERR, "u8 ", val, ""); break;
            case 2: _fputsi(STDERR, "u16 ", val, ""); break;
            case 4: _fputsi(STDERR, "u32 ", val, ""); break;
            case 8: _fputsi(STDERR, "u64 ", val, ""); break;
            default: _fputs(STDERR, "undefined");
        }
    }
    else if (value->value_type == IRNodeGlobal) {
        _assert(vsize(&value->const_arg_list) == 1);
        _fputs(STDERR, (const char*)value->const_arg_list.ptr[0]);
    }
    else{
        int sz = vsize(values_list);
        int res = -1;
        for (int i = 0; i < sz; i++) {
            if (values_list->ptr[i] == value) {
                res = i;
                break;
            }
        }

        if (res == -1) {
            res = sz;
            vpush(values_list, value);
        }

        _fputsi(STDERR, "%", res, "");
    }
}

void ir_print_block(struct Vector *blocks_list, struct IRBlock *block) {
    int sz = vsize(blocks_list);
    int res = -1;
    for (int i = 0; i < sz; i++) {
        if (blocks_list->ptr[i] == block) {
            res = i;
            break;
        }
    }

    if (res == -1) {
        res = sz;
        vpush(blocks_list, block);
    }

    _fputsi(STDERR, ".", res, "");
}

void ir_print(struct IRBuilder *builder) {
    int sz_globalvar = vsize(&builder->globalvar_list);
    for (int i = 0; i < sz_globalvar; i++) {
        struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
        _fputs2(STDERR, globalvar->name, " := ");
        if (globalvar->type == IRGlobalVarInt) {
            int value = (long)globalvar->value;
            _fputsi(STDERR, "", value, "\n");
        }
        if (globalvar->type == IRGlobalVarString) {
            const char *value = globalvar->value;
            _fputs3(STDERR, "\"", value, "\"\n");
        }
    }
    _fputs(STDERR, "\n");

    int sz_functions = vsize(&builder->function_list);
    for (int i = 0; i < sz_functions; i++) {
        struct IRFunction *function = builder->function_list.ptr[i];
        struct Vector values_list = vnew();
        struct Vector blocks_list = vnew();
        
        int sz_blocks = vsize(&function->block_list);
        int sz_args = vsize(&function->arg_list);
        if (sz_blocks) {
            _fputs(STDERR, "function ");
        }
        else {
            _fputs(STDERR, "prototype ");
        }
        _fputs2(STDERR, function->name, "(");
        for (int j = 0; j < sz_args; j++) {
            ir_print_value(&values_list, function->arg_list.ptr[j]);
            if (j + 1 != sz_args) {
                _fputs(STDERR, ", ");
            }
        }
        if (sz_blocks) {
            _fputs(STDERR, ") {\n");
        }
        else {
            _fputs(STDERR, ");\n\n");
            continue;
        }

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];
            ir_print_block(&blocks_list, block);
            _fputs(STDERR, ": ");
            int sz_pred = vsize(&block->pred_list);
            bool first = true;
            for (int k = 0; k < sz_pred; k++) {
                if (first) {
                    first = false;
                }
                else {
                    _fputs(STDERR, ", ");
                }
                ir_print_block(&blocks_list, block->pred_list.ptr[k]);
            }
            _fputs(STDERR, "\n");

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRValue *value = block->value_list.ptr[k];
                _fputs(STDERR, "    ");
                ir_print_value(&values_list, value);
                _fputs(STDERR, " := ");
                enum IRValueType type = value->value_type;
                switch (type) {
                    case IRNodeArg: _fputs(STDERR, "arg"); break;
                    case IRNodeConst: _fputs(STDERR, "const"); break;
                    case IRNodeGlobal: _fputs(STDERR, "global"); break;
                    case IRNodePhi: _fputs(STDERR, "phi"); break;
                    case IRNodeAddress: _fputs(STDERR, "address"); break;
                    case IRNodeGEP: _fputs(STDERR, "gep"); break;
                    case IRNodeSGEP: _fputs(STDERR, "sgep"); break;
                    case IRNodeAlloca: _fputs(STDERR, "alloca"); break;
                    case IRNodeLoad: _fputs(STDERR, "load"); break;
                    case IRNodeStore: _fputs(STDERR, "store"); break;
                    case IRNodeCall: _fputs(STDERR, "call"); break;
                    case IRNodeBr: _fputs(STDERR, "br"); break;
                    case IRNodeCondBr: _fputs(STDERR, "condbr"); break;
                    case IRNodeRet: _fputs(STDERR, "ret"); break;

                    case IRNodeAnd:                 _fputs(STDERR, "and"); break;
                    case IRNodeOr:                  _fputs(STDERR, "or"); break;
                    case IRNodeNot:                 _fputs(STDERR, "not"); break;
                    case IRNodeBitwiseAnd:          _fputs(STDERR, "&"); break;
                    case IRNodeBitwiseOr:           _fputs(STDERR, "|"); break;
                    case IRNodeBitwiseXor:          _fputs(STDERR, "^"); break;
                    case IRNodeBitwiseNot:          _fputs(STDERR, "~"); break;
                    case IRNodeBitwiseShiftLeft:    _fputs(STDERR, "<<"); break;
                    case IRNodeBitwiseShiftRight:   _fputs(STDERR, ">>"); break;
                    case IRNodeAddition:            _fputs(STDERR, "+"); break;
                    case IRNodeSubtraction:         _fputs(STDERR, "-"); break;
                    case IRNodeMultiplication:      _fputs(STDERR, "*"); break;
                    case IRNodeDivision:            _fputs(STDERR, "/"); break;
                    case IRNodeModulo:              _fputs(STDERR, "%"); break;
                    case IRNodeLess:                _fputs(STDERR, "<"); break;
                    case IRNodeGreater:             _fputs(STDERR, ">"); break;
                    case IRNodeEqual:               _fputs(STDERR, "=="); break;
                    case IRNodeLessEqual:           _fputs(STDERR, "<="); break;
                    case IRNodeGreaterEqual:        _fputs(STDERR, ">="); break;
                    case IRNodeNotEqual:            _fputs(STDERR, "<>"); break;
                    
                    default: _fputs(STDERR, "undefined");
                }

                bool first = true;
                int sz = vsize(&value->value_arg_list);
                for (int h = 0; h < sz; h++) {
                    if (first) {
                        _fputs(STDERR, " ");
                        first = false;
                    }
                    else {
                        _fputs(STDERR, ", ");
                    }
                    ir_print_value(&values_list, value->value_arg_list.ptr[h]);
                }

                sz = vsize(&value->block_arg_list);
                for (int h = 0; h < sz; h++) {
                    if (first) {
                        _fputs(STDERR, " ");
                        first = false;
                    }
                    else {
                        _fputs(STDERR, ", ");
                    }
                    ir_print_block(&blocks_list, value->block_arg_list.ptr[h]);
                }

                sz = vsize(&value->const_arg_list);
                for (int h = 0; h < sz; h++) {
                    if (first) {
                        _fputs(STDERR, " ");
                        first = false;
                    }
                    else {
                        _fputs(STDERR, ", ");
                    }
                    _fputsi(STDERR, "$", (long)value->const_arg_list.ptr[h], "");
                }

                _fputs(STDERR, "\n");
            }
        }

        _fputs(STDERR, "}\n\n");
    }
}

void ir_compile_type_prefix(struct TypeNode *type, int fd_text) {
    if (type->node_type == TypeNodeVoid) {
        _fputs(fd_text, "void");
    }
    else {
        if (type->node_type == TypeNodeChar) {
            _fputs(fd_text, "char");
        }
        else if (type->node_type == TypeNodeInt) {
            _fputs(fd_text, "long");
        }
        else if (type->node_type == TypeNodeFunction) {
            _fputs(fd_text, "long");
        }
        else {
            switch (type->size) {
                case -1: _panic("Unexpected type"); break;
                case 1: _fputs(fd_text, "char"); break;
                case 8: _fputs(fd_text, "long"); break;
                default: _fputs(fd_text, "char(");
            }
        }
        for (int i = 0; i < type->degree; i++) {
            _fputs(fd_text, "*");
        }
    }
    _fputs(fd_text, " ");
}

void ir_compile_type_suffix(struct TypeNode *type, int fd_text) {
    if (type->node_type == TypeNodeVoid) {
    }
    else {
        if (type->node_type == TypeNodeChar) {
        }
        else if (type->node_type == TypeNodeInt) {
        }
        else if (type->node_type == TypeNodeFunction) {
        }
        else {
            switch (type->size) {
                case -1: _panic("Unexpected type"); break;
                case 1: ; break;
                case 8: ; break;
                default: _fputsi(fd_text, ")[", type->size, "]");
            }
        }
    }
}

void ir_compile_type(struct TypeNode *type, int fd_text, bool in_parenthesis) {
    if (in_parenthesis) _fputs(fd_text, "(");
    ir_compile_type_prefix(type, fd_text);
    ir_compile_type_suffix(type, fd_text);
    if (in_parenthesis) _fputs(fd_text, ")");
}

void ir_compile_value(struct Vector *values_list, struct IRValue *value, int fd_text) {
    if (value->value_type == IRNodeConst && !value->spill) {
        _assert(vsize(&value->const_arg_list) == 2);
        int val = (long)value->const_arg_list.ptr[1];
        _fputi(fd_text, val);
    }
    else if (value->value_type == IRNodeGlobal) {
        _assert(vsize(&value->const_arg_list) == 1);
        _fputs(fd_text, (const char*)value->const_arg_list.ptr[0]);
    }
    else{
        int sz = vsize(values_list);
        int res = -1;
        for (int i = 0; i < sz; i++) {
            if (values_list->ptr[i] == value) {
                res = i;
                break;
            }
        }

        if (res == -1) {
            res = sz;
            vpush(values_list, value);
        }

        _fputsi(fd_text, "_v", res, "");
    }
}

void ir_compile_block(struct Vector *blocks_list, struct IRBlock *block, int fd_text) {
    int sz = vsize(blocks_list);
    int res = -1;
    for (int i = 0; i < sz; i++) {
        if (blocks_list->ptr[i] == block) {
            res = i;
            break;
        }
    }

    if (res == -1) {
        res = sz;
        vpush(blocks_list, block);
    }

    _fputsi(fd_text, "_b", res, "");
}

void ir_compile_phi(struct Vector *values_list, struct IRBlock *block, struct IRBlock *succ_block, int fd_text) {
    int sz_insts = vsize(&succ_block->value_list);
    for (int i = 0; i < sz_insts; i++) {
        struct IRValue *value = succ_block->value_list.ptr[i];
        if (value->value_type == IRNodePhi) {
            int sz_args = vsize(&value->value_arg_list);
            for (int j = 0; j < sz_args; j++) {
                if (block == value->block_arg_list.ptr[j] && value != value->value_arg_list.ptr[j]) {
                    _fputs(fd_text, "    ");
                    ir_compile_value(values_list, value, fd_text);
                    _fputs(fd_text, " = ");
                    ir_compile_value(values_list, value->value_arg_list.ptr[j], fd_text);
                    _fputs(fd_text, ";\n");
                }
            }
        }
    }
}

void ir_compile(struct IRBuilder *builder, const char *filename_compile_output) {
    int fd[2];
    posix_pipe(fd);
    int fd_text = fd[1];
    int fd_text_out = fd[0];

    int sz_globalvar = vsize(&builder->globalvar_list);
    for (int i = 0; i < sz_globalvar; i++) {
        struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
        ir_compile_type_prefix(globalvar->ir_value->type, fd_text);
        _fputs(fd_text, globalvar->name);
        ir_compile_type_suffix(globalvar->ir_value->type, fd_text);
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
        struct Vector values_list = vnew();
        struct Vector blocks_list = vnew();
        
        int sz_blocks = vsize(&function->block_list);
        int sz_args = vsize(&function->arg_list);

        struct TypeFunction *type_function = (struct TypeFunction*)(function->type->node_ptr);
        ir_compile_type_prefix(type_function->return_type, fd_text);
        _fputs(fd_text, function->name);
        ir_compile_type_suffix(type_function->return_type, fd_text);
        _fputs(fd_text, "(");
        for (int j = 0; j < sz_args; j++) {
            ir_compile_type_prefix(type_function->types.ptr[j], fd_text);
            ir_compile_value(&values_list, function->arg_list.ptr[j], fd_text);
            ir_compile_type_suffix(type_function->types.ptr[j], fd_text);
            if (j + 1 != sz_args) {
                _fputs(fd_text, ", ");
            }
        }
        if (sz_blocks) {
            _fputs(fd_text, ") {\n");
        }
        else {
            _fputs(fd_text, ");\n\n");
            continue;
        }

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRValue *value = block->value_list.ptr[k];
                enum IRValueType type = value->value_type;

                if (!(type == IRNodeConst && !value->spill) && 
                    type != IRNodeStore && type != IRNodeBr && 
                    type != IRNodeCondBr && type != IRNodeRet) {
                    _fputs(fd_text, "    ");
                    ir_compile_type_prefix(value->type, fd_text);
                    ir_compile_value(&values_list, value, fd_text);
                    ir_compile_type_suffix(value->type, fd_text);
                    _fputs(fd_text, ";\n");
                }
            }
        }

        for (int j = 0; j < sz_blocks; j++) {
            struct IRBlock *block = function->block_list.ptr[j];
            ir_compile_block(&blocks_list, block, fd_text);
            _fputs(fd_text, ":\n");

            int sz_insts = vsize(&block->value_list);
            for (int k = 0; k < sz_insts; k++) {
                struct IRValue *value = block->value_list.ptr[k];
                enum IRValueType type = value->value_type;
                
                if (type == IRNodeConst || type == IRNodePhi) continue;
                if (type != IRNodeStore && type != IRNodeBr && 
                    type != IRNodeCondBr && type != IRNodeRet) {
                    _fputs(fd_text, "    ");
                    ir_compile_value(&values_list, value, fd_text);
                    _fputs(fd_text, " = ");
                }

                if (type == IRNodePhi) {}
                else if (type == IRNodeAddress) {
                    _fputs(fd_text, "&");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, ";");
                }
                else if (type == IRNodeGEP) {
                    _fputs(fd_text, "&");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, "[");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[1], fd_text);
                    _fputsi(fd_text, " * ", (long)value->const_arg_list.ptr[0], "];");
                }
                else if (type == IRNodeSGEP) {
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputsi(fd_text, " + ", (long)value->const_arg_list.ptr[0], ";");
                }
                else if (type == IRNodeLoad) {
                    _fputs(fd_text, "*");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, ";");
                }
                else if (type == IRNodeStore) {
                    _fputs(fd_text, "    *");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, " = ");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[1], fd_text);
                    _fputs(fd_text, ";");
                }
                else if (type == IRNodeCall) {
                    int sz_args = vsize(&value->value_arg_list);
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, "(");
                    for (int arg = 1; arg < sz_args; arg++) {
                        ir_compile_value(&values_list, value->value_arg_list.ptr[arg], fd_text);
                        if (arg + 1 != sz_args) {
                            _fputs(fd_text, ", ");
                        }
                    }
                    _fputs(fd_text, ");");
                }
                else if (type == IRNodeBr) {
                    ir_compile_phi(&values_list, block, value->block_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, "    goto ");
                    ir_compile_block(&blocks_list, value->block_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, ";\n");
                    break;
                }
                else if (type == IRNodeCondBr) {
                    _fputs(fd_text, "    if (");
                    ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, ") {\n");
                    ir_compile_phi(&values_list, block, value->block_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, "    goto ");
                    ir_compile_block(&blocks_list, value->block_arg_list.ptr[0], fd_text);
                    _fputs(fd_text, ";\n    } else {\n");
                    ir_compile_phi(&values_list, block, value->block_arg_list.ptr[1], fd_text);
                    _fputs(fd_text, "    goto ");
                    ir_compile_block(&blocks_list, value->block_arg_list.ptr[1], fd_text);
                    _fputs(fd_text, ";\n    }\n");
                    break;
                }
                else if (type == IRNodeRet) {
                    _fputs(fd_text, "    return");
                    struct IRValue *ret_value = value->value_arg_list.ptr[0];
                    if (ret_value->type->node_type != TypeNodeVoid) {
                        _fputs(fd_text, " ");
                        ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
                    }
                    _fputs(fd_text, ";\n");
                    break;
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
                    int sz_args = vsize(&value->value_arg_list);
                    if (sz_args == 2) {
                        ir_compile_value(&values_list, value->value_arg_list.ptr[0], fd_text);
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
                    ir_compile_value(&values_list, value->value_arg_list.ptr[sz_args - 1], fd_text);
                    _fputs(fd_text, ";");
                }
                else {
                    _panic("Uncompilable value");
                }

                _fputs(fd_text, "\n");
            }
        }

        _fputs(fd_text, "}\n\n");
    }

    posix_close(fd_text);
    char *str_text = read_file_descriptor(fd_text_out);
    posix_close(fd_text_out);
    if (filename_compile_output) {
        write_file(filename_compile_output, str_text);
    }
}

void ir_build_edge(struct IRBlock *block_out, struct IRBlock *block_in) {
    vpush(&block_out->succ_list, block_in);
    vpush(&block_in->pred_list, block_out);
}

void ir_build_phi(struct IRBuilder *builder) {
    struct IRBlock *block = builder->current_block;
    int cnt_prv = vsize(&block->pred_list);
    struct IRBlock *first_prv = block->pred_list.ptr[0];
    int cnt_val = vsize(&first_prv->variable_name_list);

    for (int i = 0; i < cnt_val; i++) {
        struct IRValue *phi = ir_build_value(builder, IRNodePhi);
        bool use_phi = false;
        const char *name = first_prv->variable_name_list.ptr[i];
        struct IRValue *value = first_prv->variable_value_list.ptr[i];
        phi->type = value->type;
        bool in_all_preds = true;
        for (int j = 1; j < cnt_prv; j++) {
            struct IRBlock *prv_block = block->pred_list.ptr[j];
            int sz = vsize(&prv_block->variable_name_list);
            struct IRValue *value_this = NULL;
            for (int k = 0; k < sz; k++) {
                if (!_strcmp(name, prv_block->variable_name_list.ptr[k])) {
                    value_this = prv_block->variable_value_list.ptr[k];
                    break;
                }
            }
            
            if (!value_this) {
                in_all_preds = false;
                break;
            }
            if(value != value_this) {
                use_phi = true;
            }
            vpush(&phi->value_arg_list, value);
            vpush(&phi->block_arg_list, first_prv);
        }
        if (in_all_preds) {
            vpush(&block->variable_name_list, (char*)name);
            if (use_phi) {
                vpush(&block->variable_value_list, phi);
            }
            else {
                vpop(&block->value_list);
                vpush(&block->variable_value_list, value);
            }
        }
    }
}

struct IRValue *ir_build(struct IRBuilder *builder, struct Node *node) {
    if (node->node_type == NodeModule) {
        struct Module *_node = (struct Module*)node->node_ptr;
        
        int sz = vsize(&_node->statement_list);
        for (int i = 0; i < sz; i++) {
            ir_build(builder, _node->statement_list.ptr[i]);
        }

        return NULL;
    }
    if (node->node_type == NodeBlock) {
        struct Block *_node = (struct Block*)node->node_ptr;
        struct IRBlock *_block = ir_build_block(builder);
        struct IRValue *value_phi = ir_build_value_free(builder, IRNodePhi);
        value_phi->type = node->type;

        vpush(&builder->irblock_label_stack, (char*)_node->label);
        vpush(&builder->irblock_block_stack, _block);
        vpush(&builder->irblock_phi_stack, value_phi);

        int sz = vsize(&_node->statement_list);
        for (int i = 0; i < sz; i++) {
            ir_build(builder, _node->statement_list.ptr[i]);
        }

        struct IRValue *value_br = ir_build_value(builder, IRNodeBr);
        vpush(&value_br->block_arg_list, _block);
        ir_build_edge(builder->current_block, _block);
        builder->current_block = _block;
        if (vsize(&value_phi->value_arg_list)) {
            vpush(&_block->value_list, value_phi);
        }
        ir_build_phi(builder);

        vpop(&builder->irblock_label_stack);
        vpop(&builder->irblock_block_stack);
        vpop(&builder->irblock_phi_stack);

        return value_phi;
    }
    if (node->node_type == NodeInclude) {
        struct Include *_node = (struct Include*)node->node_ptr;
        bool old_header = builder->header;
        builder->header = true;

        int sz = vsize(&_node->statement_list);
        for (int i = 0; i < sz; i++) {
            ir_build(builder, _node->statement_list.ptr[i]);
        }

        builder->header = old_header;
        return NULL;
    }
    if (node->node_type == NodeTest) {
        _panic("Unimplemented NodeTest");
    }
    if (node->node_type == NodeIf) {
        struct If *_node = (struct If*)node->node_ptr;
        struct IRBlock *block_then = ir_build_block(builder);
        struct IRBlock *block_else = ir_build_block(builder);
        struct IRBlock *block_end = ir_build_block(builder);
        ir_build_edge(builder->current_block, block_then);
        ir_build_edge(builder->current_block, block_else);

        struct IRValue *value_condition = ir_build(builder, _node->condition_list.ptr[0]);
        struct IRValue *value_condbr = ir_build_value(builder, IRNodeCondBr);
        vpush(&value_condbr->value_arg_list, value_condition);
        vpush(&value_condbr->block_arg_list, block_then);
        vpush(&value_condbr->block_arg_list, block_else);

        builder->current_block = block_then;
        ir_build_phi(builder);
        struct IRValue *value_then = ir_build(builder, _node->block_list.ptr[0]);
        struct IRValue *value_br_then = ir_build_value(builder, IRNodeBr);
        block_then = builder->current_block;
        ir_build_edge(builder->current_block, block_end);
        vpush(&value_br_then->block_arg_list, block_end);

        builder->current_block = block_else;
        ir_build_phi(builder);
        struct IRValue *value_else = NULL;
        if (_node->else_block) {
            value_else = ir_build(builder, _node->else_block);
        }
        struct IRValue *value_br_else = ir_build_value(builder, IRNodeBr);
        block_else = builder->current_block;
        ir_build_edge(builder->current_block, block_end);
        vpush(&value_br_else->block_arg_list, block_end);

        builder->current_block = block_end;
        ir_build_phi(builder);

        if (node->type->node_type != TypeNodeVoid) {
            struct IRValue *value_phi = ir_build_value(builder, IRNodePhi);
            value_phi->type = node->type;
            vpush(&value_phi->value_arg_list, value_then);
            vpush(&value_phi->value_arg_list, value_else);
            vpush(&value_phi->block_arg_list, block_then);
            vpush(&value_phi->block_arg_list, block_else);
            return value_phi;
        }
        else {
            return NULL;
        }
    }
    if (node->node_type == NodeWhile) {
        struct While *_node = (struct While*)node->node_ptr;
        struct IRBlock *block_prev = builder->current_block;
        struct IRBlock *block_cond = ir_build_block(builder);
        struct IRBlock *block_then = ir_build_block(builder);
        struct IRBlock *block_else = ir_build_block(builder);
        struct IRBlock *block_end = ir_build_block(builder);

        struct IRValue *value_phi = ir_build_value_free(builder, IRNodePhi);
        value_phi->type = node->type;
        vpush(&builder->irloop_label_stack, (char*)_node->label);
        vpush(&builder->irloop_blockcond_stack, block_cond);
        vpush(&builder->irloop_blockend_stack, block_end);
        vpush(&builder->irloop_phi_stack, value_phi);
        
        ir_build_edge(builder->current_block, block_cond);
        struct IRValue *value_br_tocond = ir_build_value(builder, IRNodeBr);
        vpush(&value_br_tocond->block_arg_list, block_cond);

        builder->current_block = block_cond;
        ir_build_phi(builder);
        struct Vector tmp_variable_value_list = block_cond->variable_value_list;
        int sz_var = vsize(&block_cond->variable_value_list);
        for (int i = 0; i < sz_var; i++) {
            struct IRValue *old_value_phi = block_cond->variable_value_list.ptr[i];
            struct IRValue *new_value_phi = ir_build_value(builder, IRNodePhi);
            new_value_phi->type = old_value_phi->type;
            vpush(&new_value_phi->value_arg_list, tmp_variable_value_list.ptr[i]);
            vpush(&new_value_phi->block_arg_list, block_prev);
            block_cond->variable_value_list.ptr[i] = new_value_phi;
        }
        struct IRValue *value_condition = ir_build(builder, _node->condition);
        struct IRValue *value_condbr = ir_build_value(builder, IRNodeCondBr);
        vpush(&value_condbr->value_arg_list, value_condition);
        vpush(&value_condbr->block_arg_list, block_then);
        vpush(&value_condbr->block_arg_list, block_else);
        ir_build_edge(builder->current_block, block_then);
        ir_build_edge(builder->current_block, block_else);

        builder->current_block = block_then;
        ir_build_phi(builder);
        struct IRValue *value_then = ir_build(builder, _node->block);
        struct IRValue *value_br_then = ir_build_value(builder, IRNodeBr);
        ir_build_edge(builder->current_block, block_cond);
        vpush(&value_br_then->block_arg_list, block_cond);

        for (int i = 0; i < sz_var; i++) {
            _assert(!_strcmp(block_cond->variable_name_list.ptr[i], builder->current_block->variable_name_list.ptr[i]));
            struct IRValue *tmp_value_phi = block_cond->value_list.ptr[i];
            vpush(&tmp_value_phi->value_arg_list, builder->current_block->variable_value_list.ptr[i]);
            vpush(&tmp_value_phi->block_arg_list, builder->current_block);
        }

        builder->current_block = block_else;
        ir_build_phi(builder);
        struct IRValue *value_else = NULL;
        if (_node->else_block) {
            value_else = ir_build(builder, _node->else_block);
        }
        struct IRValue *value_br_else = ir_build_value(builder, IRNodeBr);
        ir_build_edge(builder->current_block, block_end);
        vpush(&value_br_else->block_arg_list, block_end);

        builder->current_block = block_end;
        ir_build_phi(builder);

        vpop(&builder->irloop_label_stack);
        vpop(&builder->irloop_blockcond_stack);
        vpop(&builder->irloop_blockend_stack);
        vpop(&builder->irloop_phi_stack);

        if (node->type->node_type != TypeNodeVoid) {
            vpush(&value_phi->value_arg_list, value_else);
            vpush(&value_phi->block_arg_list, block_else);
            vpush(&builder->current_block->value_list, value_phi);
            return value_phi;
        }
        else {
            return NULL;
        }
    }
    if (node->node_type == NodeFunctionDefinition) {
        struct FunctionDefinition *_node = (struct FunctionDefinition*)node->node_ptr;
        struct FunctionSignature *signature = _node->signature;
        struct IRFunction *function = ir_build_function(builder);
        function->name = _node->name;
        function->type = _node->type;

        struct IRValue *value = ir_build_value_free(builder, IRNodeGlobal);
        value->type = _node->type;
        vpush(&value->const_arg_list, (void*)_node->name);
        function->ir_value = value;
        
        struct IRBlock *block;
        if (!builder->header) {
            builder->current_function = function;
            block = ir_build_block(builder);
            builder->current_block = block;
        }
        
        int sz = vsize(&signature->identifiers);
        for (int i = 0; i < sz; i++) {
            struct IRValue *value = ir_build_value_free(builder, IRNodeArg);
            value->type = signature->types.ptr[i];
            vpush(&function->arg_list, value);
            if (!builder->header) {
                vpush(&block->variable_name_list, signature->identifiers.ptr[i]);
                vpush(&block->variable_value_list, value);
            }
        }
        
        if (!builder->header) {
            struct IRValue *body_value = ir_build(builder, _node->block);
            struct IRValue *ret_value = ir_build_value(builder, IRNodeRet);
            vpush(&ret_value->value_arg_list, body_value);
        }

        return NULL;
    }
    if (node->node_type == NodePrototype) {
        struct Prototype *_node = (struct Prototype*)node->node_ptr;
        struct FunctionSignature *signature = _node->signature;
        struct IRFunction *function = ir_build_function(builder);
        function->name = _node->name;
        function->type = _node->type;

        struct IRValue *value = ir_build_value_free(builder, IRNodeGlobal);
        value->type = _node->type;
        vpush(&value->const_arg_list, (void*)_node->name);
        function->ir_value = value;

        int sz = vsize(&signature->identifiers);
        for (int i = 0; i < sz; i++) {
            struct IRValue *value = ir_build_value_free(builder, IRNodeArg);
            value->type = signature->types.ptr[i];
            vpush(&function->arg_list, value);
        }

        return NULL;
    }
    if (node->node_type == NodeGlobalDefinition) {
        struct GlobalDefinition *_node = (struct GlobalDefinition*)node->node_ptr;
        struct IRGlobalVar *globalvar = (struct IRGlobalVar*)_malloc(sizeof(struct IRGlobalVar));
        globalvar->name = _node->identifier;
        globalvar->type = IRGlobalVarInt;
        
        if (_node->value) {
            globalvar->value = (void*)(long)((struct Integer*)(_node->value->node_ptr))->value;
        }
        else {
            globalvar->value = 0;
        }

        struct IRValue *value = ir_build_value_free(builder, IRNodeGlobal);
        value->type = _node->value->type;
        vpush(&value->const_arg_list, (char*)_node->identifier);
        globalvar->ir_value = value;

        vpush(&builder->globalvar_list, globalvar);

        return NULL;
    }
    if (node->node_type == NodeDefinition) {
        struct Definition *_node = (struct Definition*)node->node_ptr;
        vpush(&builder->current_block->variable_value_list, ir_build(builder, _node->value));
        vpush(&builder->current_block->variable_name_list, (char*)_node->identifier);
        
        return NULL;
    }
    if (node->node_type == NodeTypeDefinition) {
        return NULL;
    }
    if (node->node_type == NodeReturn) {
        struct Return *_node = (struct Return*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->expression);

        struct IRBlock *block = NULL;
        struct IRValue *phi = NULL;
        int sz = vsize(&builder->irblock_label_stack);
        for (int i = sz - 1; i >= 0; i--) {
            const char *label = builder->irblock_label_stack.ptr[i];
            if (!_node->label || label && !_strcmp(_node->label, label)) {
                block = builder->irblock_block_stack.ptr[i];
                phi = builder->irblock_phi_stack.ptr[i];
                break;
            }
        }
        if (!block) {
            _panic("Block not found in NodeReturn");
        }
        struct IRValue *value_br = ir_build_value(builder, IRNodeBr);
        vpush(&value_br->block_arg_list, block);
        vpush(&phi->value_arg_list, value);
        vpush(&phi->block_arg_list, builder->current_block);

        ir_build_edge(builder->current_block, block);

        return NULL;
    }
    if (node->node_type == NodeBreak) {
        struct Break *_node = (struct Break*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->expression);

        struct IRBlock *block = NULL;
        struct IRValue *phi = NULL;
        int sz = vsize(&builder->irloop_label_stack);
        for (int i = sz - 1; i >= 0; i--) {
            const char *label = builder->irloop_label_stack.ptr[i];
            if (!_node->label || label && !_strcmp(_node->label, label)) {
                block = builder->irloop_blockend_stack.ptr[i];
                phi = builder->irloop_phi_stack.ptr[i];
                break;
            }
        }
        if (!block) {
            _panic("Block not found in NodeBreak");
        }
        struct IRValue *value_br = ir_build_value(builder, IRNodeBr);
        vpush(&value_br->block_arg_list, block);
        vpush(&phi->value_arg_list, value);
        vpush(&phi->block_arg_list, builder->current_block);

        ir_build_edge(builder->current_block, block);

        return NULL;
    }
    if (node->node_type == NodeContinue) {
        struct Continue *_node = (struct Continue*)node->node_ptr;

        struct IRBlock *block = NULL;
        int sz = vsize(&builder->irloop_label_stack);
        for (int i = sz - 1; i >= 0; i--) {
            const char *label = builder->irloop_label_stack.ptr[i];
            if (!_node->label || label && !_strcmp(_node->label, label)) {
                block = builder->irloop_blockcond_stack.ptr[i];
                break;
            }
        }
        if (!block) {
            _panic("Block not found in NodeContinue");
        }
        struct IRValue *value_br = ir_build_value(builder, IRNodeBr);
        vpush(&value_br->block_arg_list, block);

        ir_build_edge(builder->current_block, block);

        return NULL;
    }
    if (node->node_type == NodeAs) {
        struct As *_node = (struct As*)node->node_ptr;
        return ir_build(builder, _node->expression);
    }
    if (node->node_type == NodeAssignment) {
        struct Assignment *_node = (struct Assignment*)node->node_ptr;
        struct Identifier *_identifier = (struct Identifier*)_node->dst->node_ptr;
        
        struct IRBlock *block = builder->current_block;
        int sz = vsize(&block->variable_name_list);
        for (int i = sz - 1; i >= 0; i--) {
            if (!_strcmp(block->variable_name_list.ptr[i], _identifier->identifier)) {
                struct IRValue *value = ir_build(builder, _node->src);
                block->variable_value_list.ptr[i] = value;
                return NULL;
            }
        }

        _panic("Identifier not found in NodeAssignment");
    }
    if (node->node_type == NodeMovement) {
        struct Assignment *_node = (struct Assignment*)node->node_ptr;
        struct IRValue *value_dst = ir_build(builder, _node->dst);
        struct IRValue *value_src = ir_build(builder, _node->src);
        struct IRValue *value_store = ir_build_value(builder, IRNodeStore);
        vpush(&value_store->value_arg_list, value_dst);
        vpush(&value_store->value_arg_list, value_src);
        vpush(&value_store->const_arg_list, (void*)(long)_node->src->type->size);

        return NULL;
    }
    if (node->node_type == NodeIdentifier) {
        struct Identifier *_node = (struct Identifier*)node->node_ptr;
        struct IRBlock *block = builder->current_block;
        
        struct IRValue *value = NULL;
        
        int sz = vsize(&builder->function_list);
        for (int i = 0; i < sz; i++) {
            struct IRFunction *function = builder->function_list.ptr[i];
            if (!_strcmp(_node->identifier, function->name)) {
                value = function->ir_value;
                break;
            }
        }

        if (!value) {
            sz = vsize(&block->variable_name_list);
            for (int i = sz - 1; i >= 0; i--) {
                if (!_strcmp(_node->identifier, block->variable_name_list.ptr[i])) {
                    value = block->variable_value_list.ptr[i];
                    break;
                }
            }
        }

        if (!value) {
            sz = vsize(&builder->globalvar_list);
            for (int i = 0; i < sz; i++) {
                struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
                if (!_strcmp(_node->identifier, globalvar->name)) {
                    value = globalvar->ir_value;
                    break;
                }
            }
        }

        if (!value) {
            _panic("Identifier node found in NodeIdentifier");
        }

        if (_node->address) {
            value->spill = true;
            struct IRValue *value_address = ir_build_value(builder, IRNodeAddress);
            vpush(&value_address->value_arg_list, value);
            value_address->type = node->type;
            return value_address;   
        }
        else {
            return value;
        }
    }
    if (node->node_type == NodeInteger) {
        struct Integer *_node = (struct Integer*)node->node_ptr;
        struct IRValue *value = ir_build_value(builder, IRNodeConst);
        value->type = node->type;
        vpush(&value->const_arg_list, (void*)8);
        vpush(&value->const_arg_list, (void*)(long)_node->value);
        return value;
    }
    if (node->node_type == NodeChar) {
        struct Char *_node = (struct Char*)node->node_ptr;
        struct IRValue *value = ir_build_value(builder, IRNodeConst);
        value->type = node->type;
        vpush(&value->const_arg_list, (void*)1);
        vpush(&value->const_arg_list, (void*)(long)_node->value);
        return value;
    }
    if (node->node_type == NodeString) {
        struct String *_node = (struct String*)node->node_ptr;
        struct IRGlobalVar *globalvar = (struct IRGlobalVar*)_malloc(sizeof(struct IRGlobalVar));
        const char *identifier = _concat("_L", _itoa(vsize(&builder->globalvar_list)));
        globalvar->name = identifier;
        globalvar->type = IRGlobalVarString;
        globalvar->value = (char*)_node->value;
        vpush(&builder->globalvar_list, globalvar);

        struct IRValue *value = ir_build_value_free(builder, IRNodeGlobal);
        value->type = node->type;
        vpush(&value->const_arg_list, (char*)identifier);
        globalvar->ir_value = value;
        return value;
    }
    if (node->node_type == NodeArray) {
        _panic("Unimplemented NodeArray");
    }
    if (node->node_type == NodeStructInstance) {
        _panic("Unimplemented NodeStructInstance");
    }
    if (node->node_type == NodeLambdaFunction) {
        _panic("Unimplemented NodeLambdaFunction");
    }
    if (node->node_type == NodeSizeof) {
        struct Sizeof *_node = (struct Sizeof*)node->node_ptr;
        struct IRValue *value = ir_build_value(builder, IRNodeConst);
        value->type = node->type;
        vpush(&value->const_arg_list, (void*)8);
        vpush(&value->const_arg_list, (void*)(long)_node->type->size);
        return value;
    }
    if (node->node_type == NodeFunctionCall) {
        struct FunctionCall *_node = (struct FunctionCall*)node->node_ptr;
        
        struct Vector arguments = vnew();
        int sz = vsize(&_node->arguments);
        vpush(&arguments, ir_build(builder, _node->function));
        for (int i = 0; i < sz; i++) {
            vpush(&arguments, ir_build(builder, _node->arguments.ptr[i]));
        }

        struct IRValue *call = ir_build_value(builder, IRNodeCall);
        call->type = node->type;
        call->value_arg_list = arguments;
        return call;
    }
    if (node->node_type == NodeMethodCall) {
        struct MethodCall *_node = (struct MethodCall*)node->node_ptr;
        
        struct Vector arguments = vnew();
        int sz = vsize(&_node->arguments);
        vpush(&arguments, _node->caller);
        for (int i = 0; i < sz; i++) {
            vpush(&arguments, ir_build(builder, _node->arguments.ptr[i]));
        }

        struct IRValue *call = ir_build_value(builder, IRNodeCall);
        call->type = node->type;
        call->value_arg_list = arguments;
        vpush(&call->const_arg_list, (void*)_node->name);
        return call;
    }
    if (node->node_type == NodeDereference) {
        struct Dereference *_node = (struct Dereference*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->expression);
        value->type = node->type;
        struct IRValue *value_load = ir_build_value(builder, IRNodeLoad);
        vpush(&value_load->value_arg_list, value);
        vpush(&value_load->const_arg_list, (void*)(long)_node->expression->type->size);
        return value_load;
    }
    if (node->node_type == NodeIndex) {
        struct Index *_node = (struct Index*)node->node_ptr;
        int size;
        if (_node->left->type->degree > 1) size = 8;
        else if(_node->left->type->degree == 1) size = node->type->size;
        else _panic("Non pointer argument in GEP");
        
        struct IRValue *value_base = ir_build(builder, _node->left);
        struct IRValue *value_index = ir_build(builder, _node->right);
        struct IRValue *value_gep = ir_build_value(builder, IRNodeGEP);
        value_gep->type = _node->left->type;
        vpush(&value_gep->value_arg_list, value_base);
        vpush(&value_gep->value_arg_list, value_index);
        vpush(&value_gep->const_arg_list, (void*)(long)size);
        
        if (_node->address) {
            return value_gep;
        }

        struct IRValue *value_load = ir_build_value(builder, IRNodeLoad);
        value_load->type = node->type;
        vpush(&value_load->value_arg_list, value_gep);
        vpush(&value_load->const_arg_list, (void*)(long)size);
        return value_load;
    }
    if (node->node_type == NodeGetField) {
        struct GetField *_node = (struct GetField*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->left);

        struct IRValue *value_sgep = ir_build_value(builder, IRNodeSGEP);
        value_sgep->type = node->type;
        vpush(&value_sgep->value_arg_list, value);
        vpush(&value_sgep->const_arg_list, (void*)(long)_node->phase);

        if (_node->address) {
            return value_sgep;
        }
        
        struct IRValue *value_load = ir_build_value(builder, IRNodeLoad);
        value_load->type = node->type;
        value_sgep->type = type_copy_node(value_sgep->type);
        value_sgep->type->degree++;
        vpush(&value_load->value_arg_list, value_sgep);
        vpush(&value_load->const_arg_list, (void*)(long)value_sgep->type->size);
        return value_load;
    }

    if (node->node_type == NodeAnd ||
        node->node_type == NodeOr ||
        node->node_type == NodeNot ||
        node->node_type == NodeBitwiseAnd ||
        node->node_type == NodeBitwiseOr ||
        node->node_type == NodeBitwiseXor ||
        node->node_type == NodeBitwiseNot ||
        node->node_type == NodeBitwiseShiftLeft ||
        node->node_type == NodeBitwiseShiftRight ||
        node->node_type == NodeAddition ||
        node->node_type == NodeSubtraction ||
        node->node_type == NodeMultiplication ||
        node->node_type == NodeDivision ||
        node->node_type == NodeModulo ||
        node->node_type == NodeLess ||
        node->node_type == NodeGreater ||
        node->node_type == NodeEqual ||
        node->node_type == NodeLessEqual ||
        node->node_type == NodeGreaterEqual ||
        node->node_type == NodeNotEqual
    ) {
        struct BinaryOperator *_node = (struct BinaryOperator*)node->node_ptr;
        struct IRValue *value1 = NULL;
        if (_node->left) {
            value1 = ir_build(builder, _node->left);
        }
        struct IRValue *value2 = ir_build(builder, _node->right);
        struct IRValue *value = ir_build_value(builder, node->node_type - NodeAnd + IRNodeAnd);
        value->type = node->type;
        if (value1) {
            vpush(&value->value_arg_list, value1);
        }
        vpush(&value->value_arg_list, value2);
        return value;
    }
    _panic("Unknows ast node");
}
