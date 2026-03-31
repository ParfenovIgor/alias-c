#include <ast.h>
#include <ir_build.h>
#include <stdlib.h>
#include <string.h>

struct IRBuilder *ir_builder(bool testing) {
    struct IRBuilder *builder = (struct IRBuilder*)_malloc(sizeof(struct IRBuilder));
    builder->current_function = NULL;
    builder->function_list = vnew();
    builder->function_stack = vnew();
    builder->globalvar_list = vnew();
    builder->nodes = vnew();
    builder->irblock_label_stack = vnew();
    builder->irblock_block_stack = vnew();
    builder->irblock_phi_stack = vnew();
    builder->irloop_label_stack = vnew();
    builder->irloop_blockend_stack = vnew();
    builder->irloop_blockcond_stack = vnew();
    builder->irloop_phi_stack = vnew();
    builder->header = false;
    builder->testing = testing;
    builder->current_identifier = 0;
    builder->test_names = vnew();
    return builder;
}

struct IRNode *ir_build_node_free(struct IRBuilder *builder, void *node_ptr, enum IRValueType node_type, struct TypeNode *type) {
    struct IRNode *node = (struct IRNode*)_malloc(sizeof(struct IRNode));
    node->node_ptr = node_ptr;
    node->node_type = node_ptr;
    node->type = type;
    return node;
}

struct IRNode *ir_build_node(struct IRBuilder *builder, void *node_ptr, enum IRValueType node_type, struct TypeNode *type) {
    struct IRNode *node = (struct IRNode*)_malloc(sizeof(struct IRNode));
    node->node_ptr = node_ptr;
    node->node_type = node_ptr;
    node->type = type;
    vpush(&builder->current_block->value_list, node);
    return node;
}

struct IRNode *ir_build_arg(struct IRBuilder *builder, struct TypeNode *type) {
    struct IRArg *arg = (struct IRArg*)_malloc(sizeif(struct IRArg));
    return ir_build_node_free(builder, arg, IRNodeArg, type);
}

struct IRNode *ir_build_const(struct IRBuilder *builder, struct TypeNode *type, long size, long value) {
    struct IRConst *_const = (struct IRConst*)_malloc(sizeif(struct IRConst));
    arg->size = size;
    arg->value = value;
    return ir_build_node(builder, _const, IRNodeConst, type);
}

struct IRNode *ir_build_global(struct IRBuilder *builder, struct TypeNode *type, const char *name) {
    struct IRGlobal *global = (struct IRGlobal*)_malloc(sizeif(struct IRGlobal));
    global->name = name;
    return ir_build_node_free(builder, global, IRNodeGlobal, type);
}

struct IRNode *ir_build_phi(struct IRBuilder *builder, struct TypeNode *type, struct Vector *values, struct Vector *blocks) {
    struct IRPhi *phi = (struct IRPhi*)_malloc(sizeif(struct IRPhi));
    phi->values = values;
    phi->blocks = blocks;
    return ir_build_node(builder, phi, IRNodePhi, type);
}

struct IRNode *ir_build_gep(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *base, struct IRNode *index, long size) {
    struct IRGEP *gep = (struct IRGEP*)_malloc(sizeif(struct IRGEP));
    gep->base = base;
    gep->index = index;
    gep->size = size;
    return ir_build_node(builder, gep, IRNodeGEP, type);
}

struct IRNode *ir_build_sgep(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *instance, long phase) {
    struct IRSGEP *sgep = (struct IRSGEP*)_malloc(sizeif(struct IRSGEP));
    sgep->instance = instance;
    sgep->phase = phase;
    return ir_build_node(builder, sgep, IRNodeSGEP, type);
}

struct IRNode *ir_build_alloca(struct IRBuilder *builder, struct TypeNode *type, long size) {
    struct IRAlloca *alloca = (struct IRAlloca*)_malloc(sizeif(struct IRAlloca));
    alloca->size = size;
    return ir_build_node(builder, alloca, IRNodeAlloca, type);
}

struct IRNode *ir_build_load(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *src, long size) {
    struct IRLoad *load = (struct IRLoad*)_malloc(sizeif(struct IRLoad));
    load->src = src;
    load->size = size;
    return ir_build_node(builder, load, IRNodeLoad, type);
}

struct IRNode *ir_build_store(struct IRBuilder *builder, struct IRNode *dst, struct IRNode *src, long size) {
    struct IRStore *store = (struct IRStore*)_malloc(sizeif(struct IRStore));
    store->dst = dst;
    store->src = src;
    store->size = size;
    return ir_build_node(builder, store, IRNodeStore, NULL);
}

struct IRNode *ir_build_call(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *function, struct Vector *arguments) {
    struct IRCall *call = (struct IRCall*)_malloc(sizeif(struct IRCall));
    call->function = function;
    call->arguments = arguments;
    return ir_build_node(builder, call, IRNodeCall, type);
}

struct IRNode *ir_build_br(struct IRBuilder *builder, struct IRBlock *block) {
    struct IRBr *br = (struct IRBr*)_malloc(sizeif(struct IRBr));
    br->block = block;
    return ir_build_node(builder, br, IRNodeBr, NULL);
}

struct IRNode *ir_build_condbr(struct IRBuilder *builder, struct IRNode *condition, struct IRBlock *block_then , struct IRBlock *block_else) {
    struct IRCondBr *condbr = (struct IRCondBr*)_malloc(sizeif(struct IRCondBr));
    condbr->condition = condition;
    condbr->block_then = block_then;
    condbr->block_else = block_else;
    return ir_build_node(builder, condbr, IRNodeCondBr, NULL);
}

struct IRNode *ir_build_ret(struct IRBuilder *builder, struct IRNode *value) {
    struct IRRet *ret = (struct IRRet*)_malloc(sizeif(struct IRRet));
    ret->value = value;
    return ir_build_node(builder, ret, IRNodeRet, NULL);
}

struct IRNode *ir_build_binary_operator(struct IRBuilder *builder, enum IRValueType *node_type, struct TypeNode *type, struct IRNode *left, struct IRNode *right) {
    struct IRBinaryOperator *binary_operator = (struct IRBinaryOperator*)_malloc(sizeif(struct IRBinaryOperator));
    binary_operator->left = left;
    binary_operator->right = right;
    return ir_build_node(builder, binary_operator, node_type, NULL);
}

struct IRNode *ir_build_value(struct IRBuilder *builder, enum IRValueType value_type) {
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
    block->variable_list = vnew();
    return block;
}

struct IRFunction *ir_build_function(struct IRBuilder *builder) {
    struct IRFunction *function = (struct IRFunction*)_malloc(sizeof(struct IRFunction));
    vpush(&builder->function_list, function);
    vpush(&builder->function_stack, function);
    function->arg_list = vnew();
    function->block_list = vnew();
    return function;
}

void ir_build_edge(struct IRBlock *block_out, struct IRBlock *block_in) {
    int cnt_succ = vsize(&block_out->succ_list);
    for (int i = 0; i < cnt_succ; i++) {
        struct IRBlock *succ_block = block_out->succ_list.ptr[i];
        if (succ_block == block_in) {
            return;
        }
    }
    vpush(&block_out->succ_list, block_in);
    vpush(&block_in->pred_list, block_out);
}

void ir_build_phi(struct IRBuilder *builder) {
    struct IRBlock *block = builder->current_block;
    int cnt_prv = vsize(&block->pred_list);
    struct IRBlock *first_prv = block->pred_list.ptr[0];
    int cnt_val = vsize(&first_prv->variable_list);

    for (int i = 0; i < cnt_val; i++) {
        struct Vector phi_values = vnew();
        struct Vector phi_blocks = vnew();
        struct TypeNode *phi_type;
        struct IRValue *phi = ir_build_value(builder, IRNodePhi);
        bool use_phi = false;
        const char *name = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->name;
        struct IRValue *value = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->value;
        bool addressed = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->addressed;
        if (addressed) {
            vpop(&block->value_list);
            vpush(&block->variable_list, first_prv->variable_list.ptr[i]);
            continue;
        }
        vpush(&phi_values, value);
        vpush(&phi_blocks, first_prv);
        phi_type = value->type;
        bool in_all_preds = true;
        for (int j = 1; j < cnt_prv; j++) {
            struct IRBlock *prv_block = block->pred_list.ptr[j];
            int sz = vsize(&prv_block->variable_list);
            struct IRValue *value_this = NULL;
            for (int k = 0; k < sz; k++) {
                if (!_strcmp(name, ((struct IRVariableInfo*)prv_block->variable_list.ptr[k])->name)) {
                    value_this = ((struct IRVariableInfo*)prv_block->variable_list.ptr[k])->value;
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
            vpush(&phi_values, value_this);
            vpush(&phi_blocks, prv_block);
        }
        if (in_all_preds) {
            struct IRVariableInfo *variable_info = (struct IRVariableInfo*)_malloc(sizeof(struct IRVariableInfo));
            variable_info->name = name;
            variable_info->addressed = false;
            if (use_phi) {
                variable_info->value = ir_build_phi(builder, phi_type, phi_values, phi_blocks);
            }
            else {
                vpop(&block->value_list);
                variable_info->value = value;
            }
            vpush(&block->variable_list, variable_info);
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
        int old_function_stack_size = vsize(&builder->function_stack);
        int old_variable_list_size = vsize(&builder->current_block->variable_list);

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
            int cnt = vsize(&builder->current_block->value_list);
            if (cnt == 0) continue;
            struct IRValue *value_last = builder->current_block->value_list.ptr[cnt - 1];
            if (value_last->node_type == IRNodeRet || 
                value_last->node_type == IRNodeBr || 
                value_last->node_type == IRNodeCondBr) {
                break;
            }
        }

        struct IRValue *value_br = ir_build_br(builder, _block);
        ir_build_edge(builder->current_block, _block);
        builder->current_block = _block;
        if (vsize(&value_phi->value_arg_list)) {
            vpush(&_block->value_list, value_phi);
        }
        ir_build_phi(builder);

        vpop(&builder->irblock_label_stack);
        vpop(&builder->irblock_block_stack);
        vpop(&builder->irblock_phi_stack);

        while (vsize(&builder->function_stack) > old_function_stack_size) {
            vpop(&builder->function_stack);
        }
        _assert(vsize(&builder->current_block->variable_list) >= old_variable_list_size);
        for (int i = 0; i < vsize(&builder->current_block->variable_list) - old_variable_list_size; i++) {
            vpop(&builder->current_block->variable_list);
        }

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
        if (!builder->testing) return NULL;

        struct Test *_node = (struct Test*)node->node_ptr;
        struct IRFunction *function = ir_build_function(builder);
        function->name = _node->name;
        function->name_generate = function->name;
        function->type = _node->type;
        function->function_definition = node;
        
        struct IRBlock *block;
        if (!builder->header) {
            builder->current_function = function;
            block = ir_build_block(builder);
            builder->current_block = block;
        }
        
        if (!builder->header) {
            struct IRValue *body_value = ir_build(builder, _node->block);
            struct IRValue *ret_value = ir_build_ret(builder, body_value);
        }

        vpush(&builder->test_names, (void*)_node->name);

        return NULL;
    }
    if (node->node_type == NodeIf) {
        struct If *_node = (struct If*)node->node_ptr;
        int cnt_branches = vsize(&_node->condition_list);
        struct IRBlock *block_end = ir_build_block(builder);
        struct IRValue *value_phi = ir_build_value_free(builder, IRNodePhi);
        value_phi->type = node->type;

        for (int i = 0; i < cnt_branches; i++) {
            struct IRBlock *block_then = ir_build_block(builder);
            struct IRBlock *block_else = ir_build_block(builder);
            ir_build_edge(builder->current_block, block_then);
            ir_build_edge(builder->current_block, block_else);

            struct IRValue *value_condition = ir_build(builder, _node->condition_list.ptr[i]);
            struct IRValue *value_condbr = ir_build_condbr(builder, value_condition, block_then, block_else);
            
            builder->current_block = block_then;
            ir_build_phi(builder);
            struct IRValue *value_then = ir_build(builder, _node->block_list.ptr[i]);
            struct IRValue *value_br_then = ir_build_br(builder, block_end);
            ir_build_edge(builder->current_block, block_end);

            if (node->type->node_type != TypeNodeVoid) {
                vpush(&value_phi->value_arg_list, value_then);
                vpush(&value_phi->block_arg_list, builder->current_block);
            }

            builder->current_block = block_else;
            ir_build_phi(builder);
        }

        struct IRValue *value_else = NULL;
        if (_node->else_block) {
            value_else = ir_build(builder, _node->else_block);
        }
        struct IRValue *value_br_else = ir_build_br(builder, block_end);
        ir_build_edge(builder->current_block, block_end);
        if (_node->else_block) {
            vpush(&value_phi->value_arg_list, value_else);
            vpush(&value_phi->block_arg_list, builder->current_block);
        }

        builder->current_block = block_end;
        ir_build_phi(builder);

        if (node->type->node_type != TypeNodeVoid) {
            vpush(&builder->current_block->value_list, value_phi);
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
        struct IRValue *value_br_tocond = ir_build_br(builder, block_cond);

        builder->current_block = block_cond;
        ir_build_phi(builder);
        struct Vector tmp_variable_list = block_cond->variable_list;
        int sz_var = vsize(&block_cond->variable_list);
        for (int i = 0; i < sz_var; i++) {
            if (((struct IRVariableInfo*)block_cond->variable_list.ptr[i])->addressed) continue;
            struct IRValue *old_value_phi = ((struct IRVariableInfo*)block_cond->variable_list.ptr[i])->value;
            struct IRValue *new_value_phi = ir_build_value(builder, IRNodePhi);
            new_value_phi->type = old_value_phi->type;
            vpush(&new_value_phi->value_arg_list, ((struct IRVariableInfo*)tmp_variable_list.ptr[i])->value);
            vpush(&new_value_phi->block_arg_list, block_prev);
            ((struct IRVariableInfo*)block_cond->variable_list.ptr[i])->value = new_value_phi;
        }
        struct IRValue *value_condition = ir_build(builder, _node->condition);
        struct IRValue *value_condbr = ir_build_condbr(builder, value_condition, block_then, block_else);
        ir_build_edge(builder->current_block, block_then);
        ir_build_edge(builder->current_block, block_else);

        builder->current_block = block_then;
        ir_build_phi(builder);
        struct IRValue *value_then = ir_build(builder, _node->block);
        struct IRValue *value_br_then = ir_build_br(builder, block_cond);
        ir_build_edge(builder->current_block, block_cond);

        int cnt = 0;
        for (int i = 0; i < sz_var; i++) {
            if (((struct IRVariableInfo*)block_cond->variable_list.ptr[i])->addressed) continue;
            _assert(!_strcmp(
                ((struct IRVariableInfo*)block_cond->variable_list.ptr[i])->name, 
                ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->name));
            struct IRValue *tmp_value_phi = block_cond->value_list.ptr[cnt];
            cnt++;
            vpush(&tmp_value_phi->value_arg_list, ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->value);
            vpush(&tmp_value_phi->block_arg_list, builder->current_block);
        }

        builder->current_block = block_else;
        ir_build_phi(builder);
        struct IRValue *value_else = NULL;
        if (_node->else_block) {
            value_else = ir_build(builder, _node->else_block);
        }
        struct IRValue *value_br_else = ir_build_br(builder, block_end);
        ir_build_edge(builder->current_block, block_end);

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
        function->function_definition = node;

        if (_node->external) {
            function->name_generate = function->name;
        }
        else {
            function->name_generate = _concat("_", _itoa(builder->current_identifier));
            builder->current_identifier++;
        }

        struct IRValue *value = ir_build_global(builder, _node->type, function->name_generate);
        function->ir_value = value;
        
        struct IRBlock *block;
        struct IRFunction *old_current_function = builder->current_function;
        struct IRBlock *old_current_block = builder->current_block;
        if (!builder->header) {
            builder->current_function = function;
            block = ir_build_block(builder);
            builder->current_block = block;
        }

        int sz = vsize(&signature->identifiers);
        for (int i = 0; i < sz; i++) {
            struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
            vpush(&function->arg_list, value);
            if (!builder->header) {
                struct IRVariableInfo *variable_info = (struct IRVariableInfo*)_malloc(sizeof(struct IRVariableInfo));
                bool addressed = ((bool*)signature->addressed.ptr)[i];
                variable_info->name = signature->identifiers.ptr[i];
                variable_info->addressed = addressed;
                if (addressed) {
                    struct IRValue *value_alloca = ir_build_value_alloca(builder, type_pointer(value->type), type_size(value->type));
                    struct IRValue *value_store = ir_build_store(builder, value->alloca, value, type_size(value->type));
                    variable_info->value = value_alloca;
                    value_alloca->spill = true;
                }
                else {
                    variable_info->value = value;
                }
                vpush(&block->variable_list, variable_info);
            }
        }
        
        if (!builder->header) {
            struct IRValue *body_value = ir_build(builder, _node->block);
            struct IRValue *ret_value = ir_build_ret(builder, body_value);
        }

        builder->current_function = old_current_function;
        builder->current_block = old_current_block;

        return NULL;
    }
    if (node->node_type == NodePrototype) {
        struct Prototype *_node = (struct Prototype*)node->node_ptr;
        struct FunctionSignature *signature = _node->signature;
        struct IRFunction *function = ir_build_function(builder);
        function->name = _node->name;
        function->name_generate = function->name;
        function->type = _node->type;
        function->function_definition = node;

        struct IRValue *value = ir_build_global(builder, _node->type, _node->name);
        function->ir_value = value;

        int sz = vsize(&signature->identifiers);
        for (int i = 0; i < sz; i++) {
            struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
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

        struct IRValue *value = ir_build_global(builder, _node->type, _node->identifier);
        globalvar->ir_value = value;

        vpush(&builder->globalvar_list, globalvar);

        return NULL;
    }
    if (node->node_type == NodeDefinition) {
        struct Definition *_node = (struct Definition*)node->node_ptr;

        struct IRValue *value;
        struct IRVariableInfo *variable_info = (struct IRVariableInfo*)_malloc(sizeof(struct IRVariableInfo));
        variable_info->name = _node->identifier;
        variable_info->addressed = _node->addressed;
        if (_node->value) {
            value = ir_build(builder, _node->value);
            if (_node->addressed || (_node->type->node_type == TypeNodeStruct && _node->type->degree == 0)) {
                struct IRValue *value_alloca = ir_build_value_alloca(builder, type_pointer(_node->type), type_size(_node->type));
                struct IRValue *value_store = ir_build_store(builder, value_alloca, value, type_size(_node->type));
                variable_info->value = value_alloca;
                value_alloca->spill = true;
                value = value_alloca;
            }
        }
        else {
            if (_node->addressed || (node->type->node_type == TypeNodeStruct && _node->type->degree == 0)) {
                value = ir_build_value_alloca(builder, type_pointer(_node->type), type_size(_node->type));
                value->spill = true;
            }
            else {
                value = ir_build_const(builder, _node->type, 8, 0);
            }
        }
        variable_info->value = value;
        vpush(&builder->current_block->variable_list, variable_info);

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
        struct IRValue *value_br = ir_build_br(builder, block);
        if (value->type->size) {
            vpush(&phi->value_arg_list, value);
        }
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
        struct IRValue *value_br = ir_build_br(builder, block);
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
        struct IRValue *value_br = ir_build_br(builder, block);

        ir_build_edge(builder->current_block, block);

        int sz_var = vsize(&block->variable_list);
        for (int i = 0; i < sz_var; i++) {
            if (((struct IRVariableInfo*)block->variable_list.ptr[i])->addressed) continue;
            _assert(!_strcmp(
                ((struct IRVariableInfo*)block->variable_list.ptr[i])->name, 
                ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->name));
            struct IRValue *tmp_value_phi = block->value_list.ptr[i];
            vpush(&tmp_value_phi->value_arg_list, ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->value);
            vpush(&tmp_value_phi->block_arg_list, builder->current_block);
        }

        return NULL;
    }
    if (node->node_type == NodeAs) {
        struct As *_node = (struct As*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->expression);
        value->type = _node->type;
        return value;
    }
    if (node->node_type == NodeAssignment) {
        struct Assignment *_node = (struct Assignment*)node->node_ptr;
        struct Identifier *_identifier = (struct Identifier*)_node->dst->node_ptr;
        
        struct IRValue *value = ir_build(builder, _node->src);
        struct IRBlock *block = builder->current_block;
        int sz = vsize(&block->variable_list);
        for (int i = sz - 1; i >= 0; i--) {
            struct IRVariableInfo *variable_info = block->variable_list.ptr[i];
            if (!_strcmp(variable_info->name, _identifier->identifier)) {
                struct IRValue *value_to = variable_info->value;
                if (value_to->spill || 
                    (_node->src->type->node_type == TypeNodeStruct && _node->src->type->degree == 0)) {
                    struct IRValue *value_store = ir_build_store(builder, value_to, value, type_size(_node->src->type));
                }
                else {
                    variable_info->value = value;
                }
                return NULL;
            }
        }
        
        sz = vsize(&builder->globalvar_list);
        for (int i = 0; i < sz; i++) {
            struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
            if (!_strcmp(globalvar->name, _identifier->identifier)) {
                {
                    struct IRValue *value_store = ir_build_store(builder, globalvar->ir_value, value, type_size(_node->src->type));
                }
                return NULL;
            }
        }

        _panic("Identifier not found in NodeAssignment");
    }
    if (node->node_type == NodeMovement) {
        struct Assignment *_node = (struct Assignment*)node->node_ptr;
        struct IRValue *value_dst = ir_build(builder, _node->dst);
        struct IRValue *value_src = ir_build(builder, _node->src);
        struct IRValue *value_store = ir_build_store(builder, value_dst, value_src, type_size(_node->src->type));
        return NULL;
    }
    if (node->node_type == NodeIdentifier) {
        struct Identifier *_node = (struct Identifier*)node->node_ptr;
        struct IRBlock *block = builder->current_block;
        
        struct IRValue *value = NULL;
        
        int sz = vsize(&builder->function_stack);
        for (int i = sz - 1; i >= 0; i--) {
            struct IRFunction *function = builder->function_stack.ptr[i];
            if (!_strcmp(_node->identifier, function->name)) {
                value = function->ir_value;
                break;
            }
        }

        if (!value) {
            int sz = vsize(&block->variable_list);
            for (int i = sz - 1; i >= 0; i--) {
                struct IRVariableInfo *variable_info = block->variable_list.ptr[i];
                if (!_strcmp(_node->identifier, variable_info->name)) {
                    if (variable_info->addressed && !_node->address && 
                        !(node->type->node_type == TypeNodeStruct && node->type->degree == 0)) {
                        struct IRValue *value_load = ir_build_load(builder, type_deref(variable_info->value->type), variable_info->value, type_size(node->type));
                        value = value_load;
                    }
                    else {
                        value = variable_info->value;
                    }
                    break;
                }
            }
        }

        if (!value) {
            int sz = vsize(&builder->globalvar_list);
            for (int i = 0; i < sz; i++) {
                struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
                if (!_strcmp(_node->identifier, globalvar->name)) {
                    value = globalvar->ir_value;
                    if (_node->address) {
                        return value;
                    }
                    else {
                        struct IRValue *value_address = ir_build_load(builder, node->type, value, type_size(node->type));
                        return value_address;
                    }
                }
            }
        }

        if (!value) {
            _panic("Identifier not found in NodeIdentifier");
        }

        return value;
    }
    if (node->node_type == NodeInteger) {
        struct Integer *_node = (struct Integer*)node->node_ptr;
        struct IRValue *value = ir_build_const(builder, node->type, 8, _node->value);
        return value;
    }
    if (node->node_type == NodeChar) {
        struct Char *_node = (struct Char*)node->node_ptr;
        struct IRValue *value = ir_build_const(builder, node->type, 1, _node->value);
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

        struct IRValue *value_global = ir_build_global(builder, node->type, identifier);
        globalvar->ir_value = value_global;

        struct IRValue *value_load = ir_build_load(builder, node->type, value_global, 8);

        return value_load;
    }
    if (node->node_type == NodeArray) {
        _panic("Unimplemented NodeArray");
    }
    if (node->node_type == NodeStructInstance) {
        struct StructInstance *_node = (struct StructInstance*)node->node_ptr;
        struct IRValue *value_alloca = ir_build_alloca(builder, type_pointer(node->type), type_size(node->type));

        struct TypeStruct *type_struct = (struct TypeStruct*)value_alloca->type->node_ptr;
        int sz = vsize(&type_struct->types);
        int phase = 0;
        for (int i = 0; i < sz; i++) {
            struct TypeNode *type = (struct TypeNode*)type_struct->types.ptr[i];
            struct TypeNode *sgep_type = type_copy_node(type);
            sgep_type->degree++;

            struct IRValue *value = ir_build(builder, (struct Node*)_node->values.ptr[i]);
            struct IRValue *value_sgep = ir_build_value_sgep(builder, sgep_type, value_alloca, phase);
            struct IRValue *value_store = ir_build_store(builder, value_sgep, value, type_size(type));

            phase += type->size;
        }

        return value_alloca;
    }
    if (node->node_type == NodeLambdaFunction) {
        struct LambdaFunction *_node = (struct LambdaFunction*)node->node_ptr;
        struct FunctionSignature *signature = _node->signature;
        struct IRFunction *function = ir_build_function(builder);
        function->type = node->type;
        function->function_definition = node;

        function->name = _concat("_", _itoa(builder->current_identifier));
        function->name_generate = function->name;
        builder->current_identifier++;

        struct IRValue *value = ir_build_global(builder, node->type, function->name_generate);
        function->ir_value = value;
        
        struct IRBlock *block;
        struct IRFunction *old_current_function = builder->current_function;
        struct IRBlock *old_current_block = builder->current_block;

        builder->current_function = function;
        block = ir_build_block(builder);
        builder->current_block = block;

        int sz = vsize(&signature->identifiers);
        for (int i = 0; i < sz; i++) {
            struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
            vpush(&function->arg_list, value);
            if (!builder->header) {
                struct IRVariableInfo *variable_info = (struct IRVariableInfo*)_malloc(sizeof(struct IRVariableInfo));
                bool addressed = ((bool*)signature->addressed.ptr)[i];
                variable_info->name = signature->identifiers.ptr[i];
                variable_info->addressed = addressed;
                if (addressed) {
                    struct IRValue *value_alloca = ir_build_alloca(builder, type_pointer(node->type), type_size(value->type));
                    struct IRValue *value_store = ir_build_store(builder, value_alloca, value, type_size(value->type));
                    variable_info->value = value_alloca;
                    value_alloca->spill = true;
                }
                else {
                    variable_info->value = value;
                }
                vpush(&block->variable_list, variable_info);
            }
        }

        struct IRValue *body_value = ir_build(builder, _node->block);
        struct IRValue *ret_value = ir_build_ret(builder, body_value);

        builder->current_function = old_current_function;
        builder->current_block = old_current_block;

        return value;
    }
    if (node->node_type == NodeSizeof) {
        struct Sizeof *_node = (struct Sizeof*)node->node_ptr;
        struct IRValue *value = ir_build_const(builder, node->type, 8, type_get_size(_node->type));
        return value;
    }
    if (node->node_type == NodeFunctionCall) {
        struct FunctionCall *_node = (struct FunctionCall*)node->node_ptr;
        
        struct IRNode *value_function = ir_build(builder, _node->function);
        struct Vector arguments = vnew();
        int sz = vsize(&_node->arguments);
        for (int i = 0; i < sz; i++) {
            vpush(&arguments, ir_build(builder, _node->arguments.ptr[i]));
        }

        struct IRValue *call = ir_build_call(builder, node->type, value_function, arguments);
        return call;
    }
    if (node->node_type == NodeMethodCall) {
        struct MethodCall *_node = (struct MethodCall*)node->node_ptr;
        
        struct IRNode *value_function = NULL;
        bool found = false;
        int sz = vsize(&builder->function_list);
        for (int i = 0; i < sz; i++) {
            struct IRFunction *function = builder->function_list.ptr[i];
            if (function->function_definition == _node->function_definition) {
                value_function = function->ir_value;
                break;
            }
        }
        _assert(value_function);
        struct Vector arguments = vnew();
        sz = vsize(&_node->arguments);
        vpush(&arguments, ir_build(builder, _node->caller));
        for (int i = 0; i < sz; i++) {
            vpush(&arguments, ir_build(builder, _node->arguments.ptr[i]));
        }

        struct IRValue *call = ir_build_call(builder, node->type, value_function, arguments);
        // vpush(&call->const_arg_list, (void*)_node->name);
        return call;
    }
    if (node->node_type == NodeDereference) {
        struct Dereference *_node = (struct Dereference*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->expression);
        struct IRValue *value_load = ir_build_load(builder, type_deref(node->type), value, type_size(_node->expression->type));
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
        struct IRValue *value_gep = ir_build_gep(builder, _node->left->type, value_base, value_index, size);
        if (_node->address) {
            return value_gep;
        }

        struct IRValue *value_load = ir_build_load(builder, node->type, value_gep, size);
        return value_load;
    }
    if (node->node_type == NodeGetField) {
        struct GetField *_node = (struct GetField*)node->node_ptr;
        struct IRValue *value = ir_build(builder, _node->left);

        struct IRValue *value_sgep = ir_build_value_sgep(builder, node->type, value, _node->phase);
        if ((node->type->node_type == TypeNodeStruct && node->type->degree == 0)) {
            value_sgep->type = type_copy_node(value_sgep->type);
            value_sgep->type->degree++;
            return value_sgep;
        }
        if (_node->address) {
            return value_sgep;
        }
        
        value_sgep->type = type_copy_node(value_sgep->type);
        value_sgep->type->degree++;
        
        struct IRValue *value_load = ir_build_load(builder, node->type, node->type, value_sgep, type_size(value_sgep->type));
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
            if (value1->spill) {
                struct IRValue *value_load = ir_build_load(builder, _node->left->type, value1, type_size(_node->left->type));
                value1 = value_load;
            }
        }

        struct IRValue *value2 = ir_build(builder, _node->right);
        if (value2->spill) {
            struct IRValue *value_load = ir_build_load(builder, _node->right->type, value2, type_size(_node->right->type));
            value2 = value_load;
        }

        struct IRValue *value = ir_build_binary_operator(builder, node->node_type - NodeAnd + IRNodeAnd, node->type, value1, value2);
        return value;
    }
    _panic("Unknown ast node");
}
