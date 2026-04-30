#include <ast.h>
#include <ir_build.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <posix.h>

#define Alloc(T, x) struct T *x = (struct T*)_malloc(sizeof(struct T));

struct IRNode *ir_create_node_free(struct IRBuilder *builder, void *node_ptr, enum IRNodeType node_type, struct TypeNode *type) {
    Alloc(IRNode, node);
    node->idx = builder->value_idx;
    builder->value_idx++;
    node->node_ptr = node_ptr;
    node->node_type = node_type;
    node->type = type;
    node->reg = REGNONE;
    node->spill = false;
    node->uses = vnew();
    node->uses_address = vnew();
    node->block = builder->current_block;
    node->loop_degree = builder->loop_degree;
    if (node->block) node->right_bound = node->block->idx;
    return node;
}

struct IRNode *ir_create_node(struct IRBuilder *builder, void *node_ptr, enum IRNodeType node_type, struct TypeNode *type) {
    struct IRNode *node = ir_create_node_free(builder, node_ptr, node_type, type);
    vpush(&builder->current_block->value_list, node);
    return node;
}

void ir_add_node_arg(struct IRNode *node, struct IRNode **arg, struct IRNode *used) {
    *arg = used;
    if (used) {
        vpush(&used->uses, node);
        vpush(&used->uses_address, arg);
    }
}

void ir_add_node_args(struct IRNode *node, struct Vector *arg, struct Vector used) {
    *arg = used;
    int sz = vsize(&used);
    for (int i = 0; i < sz; i++) {
        struct IRNode *_used = used.ptr[i];
        vpush(&_used->uses, node);
        vpush(&_used->uses_address, &used.ptr[i]);
    }
}

struct IRNode *ir_build_arg(struct IRBuilder *builder, struct TypeNode *type) {
    Alloc(IRArg, arg);
    return ir_create_node_free(builder, arg, IRNodeArg, type);
}

struct IRNode *ir_build_const(struct IRBuilder *builder, struct TypeNode *type, long size, long value) {
    Alloc(IRConst, _const);
    _const->size = size;
    _const->value = value;
    return ir_create_node_free(builder, _const, IRNodeConst, type);
}

struct IRNode *ir_build_global(struct IRBuilder *builder, struct TypeNode *type, const char *name) {
    Alloc(IRGlobal, global);
    global->name = name;
    return ir_create_node_free(builder, global, IRNodeGlobal, type);
}

struct IRNode *ir_build_phi(struct IRBuilder *builder, struct TypeNode *type, struct Vector values, struct Vector blocks) {
    Alloc(IRPhi, phi);
    struct IRNode *node = ir_create_node(builder, phi, IRNodePhi, type);
    ir_add_node_args(node, &phi->values, values);
    phi->blocks = blocks;
    return node;
}

struct IRNode *ir_build_gep(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *base, struct IRNode *index, long size) {
    Alloc(IRGEP, gep);
    struct IRNode *node = ir_create_node(builder, gep, IRNodeGEP, type);
    ir_add_node_arg(node, &gep->base, base);
    ir_add_node_arg(node, &gep->index, index);
    gep->size = size;
    return node;
}

struct IRNode *ir_build_sgep(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *instance, long phase) {
    Alloc(IRSGEP, sgep);
    struct IRNode *node = ir_create_node(builder, sgep, IRNodeSGEP, type);
    ir_add_node_arg(node, &sgep->instance, instance);
    sgep->phase = phase;
    return node;
}

struct IRNode *ir_build_alloca(struct IRBuilder *builder, struct TypeNode *type, long size) {
    Alloc(IRAlloca, alloca);
    alloca->size = size;
    return ir_create_node(builder, alloca, IRNodeAlloca, type);
}

struct IRNode *ir_build_load(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *src, long size) {
    Alloc(IRLoad, load);
    struct IRNode *node = ir_create_node(builder, load, IRNodeLoad, type);
    ir_add_node_arg(node, &load->src, src);
    load->size = size;
    return node;
}

struct IRNode *ir_build_store(struct IRBuilder *builder, struct IRNode *dst, struct IRNode *src, long size) {
    Alloc(IRStore, store);
    struct IRNode *node = ir_create_node(builder, store, IRNodeStore, NULL);
    ir_add_node_arg(node, &store->dst, dst);
    ir_add_node_arg(node, &store->src, src);
    store->size = size;
    return node;
}

struct IRNode *ir_build_call(struct IRBuilder *builder, struct TypeNode *type, struct IRNode *function, struct Vector arguments) {
    Alloc(IRCall, call);
    struct IRNode *node = ir_create_node(builder, call, IRNodeCall, type);
    ir_add_node_arg(node, &call->function, function);
    ir_add_node_args(node, &call->arguments, arguments);
    return node;
}

struct IRNode *ir_build_br(struct IRBuilder *builder, struct IRBlock *block) {
    Alloc(IRBr, br);
    br->block = block;
    return ir_create_node(builder, br, IRNodeBr, NULL);
}

struct IRNode *ir_build_condbr(struct IRBuilder *builder, struct IRNode *condition, struct IRBlock *block_then , struct IRBlock *block_else) {
    Alloc(IRCondBr, condbr);
    struct IRNode *node = ir_create_node(builder, condbr, IRNodeCondBr, NULL);
    ir_add_node_arg(node, &condbr->condition, condition);
    condbr->block_then = block_then;
    condbr->block_else = block_else;
    return node;
}

struct IRNode *ir_build_ret(struct IRBuilder *builder, struct IRNode *value) {
    Alloc(IRRet, ret);
    struct IRNode *node = ir_create_node(builder, ret, IRNodeRet, NULL);
    ir_add_node_arg(node, &ret->value, value);
    return node;
}

struct IRNode *ir_build_binary_operator(struct IRBuilder *builder, enum IRNodeType node_type, struct TypeNode *type, struct IRNode *left, struct IRNode *right) {
    Alloc(IRBinaryOperator, binary_operator);
    struct IRNode *node = ir_create_node(builder, binary_operator, node_type, type);
    ir_add_node_arg(node, &binary_operator->left, left);
    ir_add_node_arg(node, &binary_operator->right, right);
    return node;
}

struct IRBlock *ir_create_block(struct IRBuilder *builder) {
    Alloc(IRBlock, block);
    vpush(&builder->current_function->block_list, block);
    block->idx = builder->block_idx;
    builder->block_idx++;
    block->value_list = vnew();
    block->succ_list = vnew();
    block->pred_list = vnew();
    block->variable_list = vnew();
    return block;
}

struct IRBlock *ir_create_block_free(struct IRBuilder *builder) {
    Alloc(IRBlock, block);
    block->value_list = vnew();
    block->succ_list = vnew();
    block->pred_list = vnew();
    block->variable_list = vnew();
    return block;
}

void ir_assign_free_block(struct IRBuilder *builder, struct IRBlock *block) {
    vpush(&builder->current_function->block_list, block);
    block->idx = builder->block_idx;
    builder->block_idx++;
}

struct IRFunction *ir_create_function(struct IRBuilder *builder) {
    Alloc(IRFunction, function);
    vpush(&builder->function_list, function);
    vpush(&builder->function_stack, function);
    function->arg_list = vnew();
    function->block_list = vnew();
    return function;
}

struct IRGlobalVar *ir_build_globalvar(struct IRBuilder *builder, const char *name, enum IRGlobalVarType type, void *const_value, struct IRNode *ir_value) {
    Alloc(IRGlobalVar, globalvar);
    globalvar->name = name;
    globalvar->type = type;
    globalvar->const_value = const_value;
    globalvar->ir_value = ir_value;
    return globalvar;
}

void ir_create_edge(struct IRBlock *block_out, struct IRBlock *block_in) {
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

void ir_create_block_transitions(struct IRBuilder *builder) {
    struct IRBlock *block = builder->current_block;
    int cnt_prv = vsize(&block->pred_list);
    struct IRBlock *first_prv = block->pred_list.ptr[0];
    int cnt_val = vsize(&first_prv->variable_list);

    for (int i = 0; i < cnt_val; i++) {
        struct Vector phi_values = vnew();
        struct Vector phi_blocks = vnew();
        struct TypeNode *phi_type;
        bool use_phi = false;
        const char *name = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->name;
        struct IRNode *value = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->value;
        bool addressed = ((struct IRVariableInfo*)first_prv->variable_list.ptr[i])->addressed;
        if (addressed) {
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
            struct IRNode *value_this = NULL;
            for (int k = sz - 1; k >= 0; k--) {
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
            Alloc(IRVariableInfo, variable_info);
            variable_info->name = name;
            variable_info->addressed = false;
            if (use_phi) {
                variable_info->value = ir_build_phi(builder, phi_type, phi_values, phi_blocks);
            }
            else {
                variable_info->value = value;
            }
            vpush(&block->variable_list, variable_info);
        }
    }
}

void ir_build_module(struct IRBuilder *builder, struct Node *node, struct Module *this) {
    int sz = vsize(&this->statement_list);
    for (int i = 0; i < sz; i++) {
        ir_build(builder, this->statement_list.ptr[i]);
    }
    while (vsize(&builder->nodes)) {
        struct Node *_node = vback(&builder->nodes);
        vpop(&builder->nodes);
        ir_build(builder, _node);
    }
}

struct IRNode *ir_build_block(struct IRBuilder *builder, struct Node *node, struct Block *this) {
    int old_function_stack_size = vsize(&builder->function_stack);
    int old_variable_list_size = vsize(&builder->current_block->variable_list);

    struct IRBlock *_block = ir_create_block_free(builder);

    struct Vector values = vnew();
    struct Vector blocks = vnew();
    vpush(&builder->irblock_label_stack, (char*)this->label);
    vpush(&builder->irblock_block_stack, _block);
    vpush(&builder->irblock_phi_stack_values, &values);
    vpush(&builder->irblock_phi_stack_blocks, &blocks);

    int sz = vsize(&this->statement_list);
    for (int i = 0; i < sz; i++) {
        ir_build(builder, this->statement_list.ptr[i]);
        int cnt = vsize(&builder->current_block->value_list);
        if (cnt == 0) continue;
        struct IRNode *value_last = builder->current_block->value_list.ptr[cnt - 1];
        if (value_last->node_type == IRNodeRet || 
            value_last->node_type == IRNodeBr || 
            value_last->node_type == IRNodeCondBr) {
            break;
        }
    }

    struct IRNode *value_br = ir_build_br(builder, _block);
    ir_create_edge(builder->current_block, _block);
    builder->current_block = _block;
    ir_assign_free_block(builder, _block);
    struct IRNode *return_value = NULL;
    if (node->type->node_type != TypeNodeVoid) {
        return_value = ir_build_phi(builder, node->type, values, blocks);
    }
    ir_create_block_transitions(builder);

    vpop(&builder->irblock_label_stack);
    vpop(&builder->irblock_block_stack);
    vpop(&builder->irblock_phi_stack_values);
    vpop(&builder->irblock_phi_stack_blocks);

    while (vsize(&builder->function_stack) > old_function_stack_size) {
        vpop(&builder->function_stack);
    }
    _assert(vsize(&builder->current_block->variable_list) >= old_variable_list_size);
    for (int i = 0; i < vsize(&builder->current_block->variable_list) - old_variable_list_size; i++) {
        vpop(&builder->current_block->variable_list);
    }

    return return_value;
}

void ir_build_include(struct IRBuilder *builder, struct Node *node, struct Include *this) {
    bool old_header = builder->header;
    builder->header = true;

    int sz = vsize(&this->statement_list);
    for (int i = 0; i < sz; i++) {
        ir_build(builder, this->statement_list.ptr[i]);
    }

    builder->header = old_header;
}

void ir_build_test(struct IRBuilder *builder, struct Node *node, struct Test *this) {
    if (!builder->testing) return;
    int old_block_idx = builder->block_idx;
    int old_value_idx = builder->value_idx;
    builder->block_idx = 0;
    builder->value_idx = 0;

    struct IRFunction *function = ir_create_function(builder);
    function->name = this->name;
    function->name_generate = function->name;
    function->type = this->type;
    function->function_definition = node;
    
    struct IRBlock *block;
    if (!builder->header) {
        builder->current_function = function;
        block = ir_create_block(builder);
        builder->current_block = block;
    }
    
    if (!builder->header) {
        struct IRNode *body_value = ir_build(builder, this->block);
        struct IRNode *ret_value = ir_build_ret(builder, body_value);
    }

    vpush(&builder->test_names, (void*)this->name);

    builder->value_idx = old_value_idx;
    builder->block_idx = old_block_idx;
}

struct IRNode *ir_build_if(struct IRBuilder *builder, struct Node *node, struct If *this) {
    int cnt_branches = vsize(&this->condition_list);
    struct IRBlock *block_end = ir_create_block_free(builder);
    struct Vector values = vnew();
    struct Vector blocks = vnew();

    for (int i = 0; i < cnt_branches; i++) {
        struct IRBlock *block_then = ir_create_block_free(builder);
        struct IRBlock *block_else = ir_create_block_free(builder);
        ir_create_edge(builder->current_block, block_then);
        ir_create_edge(builder->current_block, block_else);

        struct IRNode *value_condition = ir_build(builder, this->condition_list.ptr[i]);
        struct IRNode *value_condbr = ir_build_condbr(builder, value_condition, block_then, block_else);
        
        builder->current_block = block_then;
        ir_assign_free_block(builder, block_then);
        ir_create_block_transitions(builder);
        struct IRNode *value_then = ir_build(builder, this->block_list.ptr[i]);
        struct IRNode *value_br_then = ir_build_br(builder, block_end);
        ir_create_edge(builder->current_block, block_end);

        if (node->type->node_type != TypeNodeVoid) {
            vpush(&values, value_then);
            vpush(&blocks, builder->current_block);
        }

        builder->current_block = block_else;
        ir_assign_free_block(builder, block_else);
        ir_create_block_transitions(builder);
    }

    struct IRNode *value_else = NULL;
    if (this->else_block) {
        value_else = ir_build(builder, this->else_block);
    }
    struct IRNode *value_br_else = ir_build_br(builder, block_end);
    ir_create_edge(builder->current_block, block_end);
    if (this->else_block) {
        vpush(&values, value_else);
        vpush(&blocks, builder->current_block);
    }

    builder->current_block = block_end;
    ir_assign_free_block(builder, block_end);
    ir_create_block_transitions(builder);

    if (node->type->node_type != TypeNodeVoid) {
        return ir_build_phi(builder, node->type, values, blocks);
    }
    else {
        return NULL;
    }
}

struct IRNode *ir_build_while(struct IRBuilder *builder, struct Node *node, struct While *this) {
    builder->loop_degree++;
    struct IRBlock *block_prev = builder->current_block;
    struct IRBlock *block_cond = ir_create_block_free(builder);
    struct IRBlock *block_then = ir_create_block_free(builder);
    struct IRBlock *block_else = ir_create_block_free(builder);
    struct IRBlock *block_end = ir_create_block_free(builder);

    struct Vector values = vnew();
    struct Vector blocks = vnew();
    vpush(&builder->irloop_label_stack, (char*)this->label);
    vpush(&builder->irloop_blockcond_stack, block_cond);
    vpush(&builder->irloop_blockend_stack, block_end);
    vpush(&builder->irloop_phi_stack_values, &values);
    vpush(&builder->irloop_phi_stack_blocks, &blocks);
    
    ir_create_edge(builder->current_block, block_cond);
    struct IRNode *value_br_tocond = ir_build_br(builder, block_cond);

    builder->current_block = block_cond;
    ir_assign_free_block(builder, block_cond);
    ir_create_block_transitions(builder);
    struct Vector tmp_variable_list = block_cond->variable_list;
    int sz_var = vsize(&block_cond->variable_list);
    for (int i = 0; i < sz_var; i++) {
        struct IRVariableInfo *variable = block_cond->variable_list.ptr[i];
        if (variable->addressed) continue;
        struct IRNode *old_value_phi = variable->value;
        struct Vector values = vnew();
        struct Vector blocks = vnew();
        vpush(&values, old_value_phi);
        vpush(&blocks, block_prev);
        struct IRNode *new_value_phi = ir_build_phi(builder, old_value_phi->type, values, blocks);
        variable->value = new_value_phi;
    }
    struct IRNode *value_condition = ir_build(builder, this->condition);
    struct IRNode *value_condbr = ir_build_condbr(builder, value_condition, block_then, block_else);
    ir_create_edge(builder->current_block, block_then);
    ir_create_edge(builder->current_block, block_else);

    builder->current_block = block_then;
    ir_assign_free_block(builder, block_then);
    ir_create_block_transitions(builder);
    struct IRNode *value_then = ir_build(builder, this->block);
    struct IRNode *value_br_then = ir_build_br(builder, block_cond);
    ir_create_edge(builder->current_block, block_cond);

    int cnt = 0;
    for (int i = 0; i < sz_var; i++) {
        struct IRVariableInfo *variable_original = block_prev->variable_list.ptr[i];
        struct IRVariableInfo *variable_before = block_cond->variable_list.ptr[i];
        struct IRVariableInfo *variable_after = builder->current_block->variable_list.ptr[i];
        if (variable_before->addressed) continue;
        _assert(!_strcmp(
            variable_before->name, 
            variable_after->name));
        struct IRPhi *tmp_value_phi = ((struct IRNode*)block_cond->value_list.ptr[cnt])->node_ptr;
        cnt++;
        if (variable_before->value == variable_after->value) {
            int cnt_uses = vsize(&variable_before->value->uses);
            for (int j = 0; j < cnt_uses; j++) {
                struct IRNode **value = variable_before->value->uses_address.ptr[j];
                *value = variable_original->value;
                vpush(&variable_original->value->uses, variable_before->value->uses.ptr[j]);
                vpush(&variable_original->value->uses_address, variable_before->value->uses_address.ptr[j]);
            }
            vpop(&tmp_value_phi->values);
            vpop(&tmp_value_phi->blocks);
            variable_before->value = variable_original->value;
            variable_after->value = variable_original->value;
            variable_original->value->right_bound = builder->current_block->idx;
        }
        else {
            vpush(&tmp_value_phi->values, variable_after->value);
            vpush(&tmp_value_phi->blocks, builder->current_block);
        }
    }

    builder->current_block = block_else;
    ir_assign_free_block(builder, block_else);
    ir_create_block_transitions(builder);
    struct IRNode *value_else = NULL;
    if (this->else_block) {
        value_else = ir_build(builder, this->else_block);
    }
    struct IRNode *value_br_else = ir_build_br(builder, block_end);
    ir_create_edge(builder->current_block, block_end);

    builder->current_block = block_end;
    ir_assign_free_block(builder, block_end);

    struct IRNode *return_value = NULL;
    if (node->type->node_type != TypeNodeVoid) {
        vpush(&values, value_else);
        vpush(&blocks, block_else);
        return_value = ir_build_phi(builder, node->type, values, blocks);
    }

    ir_create_block_transitions(builder);

    vpop(&builder->irloop_label_stack);
    vpop(&builder->irloop_blockcond_stack);
    vpop(&builder->irloop_blockend_stack);
    vpop(&builder->irloop_phi_stack_values);
    vpop(&builder->irloop_phi_stack_blocks);

    builder->loop_degree--;
    return return_value;
}

void ir_build_function_definition(struct IRBuilder *builder, struct Node *node, struct FunctionDefinition *this) {
    int old_block_idx = builder->block_idx;
    int old_value_idx = builder->value_idx;
    builder->block_idx = 0;
    builder->value_idx = 0;

    struct FunctionSignature *signature = this->signature;
    struct IRFunction *function = ir_create_function(builder);
    function->name = this->name;
    function->type = this->type;
    function->function_definition = node;

    if (this->external) {
        function->name_generate = function->name;
    }
    else {
        function->name_generate = _concat("_", _itoa(builder->current_identifier));
        builder->current_identifier++;
    }

    struct IRNode *value = ir_build_global(builder, this->type, function->name_generate);
    function->ir_value = value;
    
    struct IRBlock *block;
    struct IRFunction *old_current_function = builder->current_function;
    struct IRBlock *old_current_block = builder->current_block;
    if (!builder->header) {
        builder->current_function = function;
        block = ir_create_block(builder);
        builder->current_block = block;
    }

    int sz = vsize(&signature->identifiers);
    for (int i = 0; i < sz; i++) {
        struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
        vpush(&function->arg_list, value);
        if (!builder->header) {
            Alloc(IRVariableInfo, variable_info);
            bool addressed = ((bool*)signature->addressed.ptr)[i];
            variable_info->name = signature->identifiers.ptr[i];
            variable_info->addressed = addressed;
            if (addressed) {
                struct IRNode *value_alloca = ir_build_alloca(builder, type_pointer(value->type), type_size(value->type));
                struct IRNode *value_store = ir_build_store(builder, value_alloca, value, type_size(value->type));
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
        struct IRNode *body_value = ir_build(builder, this->block);
        struct IRNode *ret_value = ir_build_ret(builder, body_value);
    }

    builder->current_function = old_current_function;
    builder->current_block = old_current_block;

    builder->value_idx = old_value_idx;
    builder->block_idx = old_block_idx;
}

void ir_build_prototype(struct IRBuilder *builder, struct Node *node, struct Prototype *this) {
    struct FunctionSignature *signature = this->signature;
    struct IRFunction *function = ir_create_function(builder);
    function->name = this->name;
    function->name_generate = function->name;
    function->type = this->type;
    function->function_definition = node;

    struct IRNode *value = ir_build_global(builder, this->type, this->name);
    function->ir_value = value;

    int sz = vsize(&signature->identifiers);
    for (int i = 0; i < sz; i++) {
        struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
        vpush(&function->arg_list, value);
    }
}

void ir_build_global_definition(struct IRBuilder *builder, struct Node *node, struct GlobalDefinition *this) {
    long const_value = 0;
    if (this->value) {
        const_value = ((struct Integer*)(this->value->node_ptr))->value;
    }
    struct IRNode *ir_value = ir_build_global(builder, this->type, this->identifier);
    vpush(&builder->globalvar_list, ir_build_globalvar(builder, this->identifier, IRGlobalVarInt, (void*)const_value, ir_value));
}

void ir_build_definition(struct IRBuilder *builder, struct Node *node, struct Definition *this) {
    Alloc(IRVariableInfo, variable_info);
    struct IRNode *value;
    variable_info->name = this->identifier;
    variable_info->addressed = this->addressed;
    if (this->value) {
        value = ir_build(builder, this->value);
        if (this->addressed || ir_value_is_complex(this->type)) {
            struct IRNode *value_alloca = ir_build_alloca(builder, type_pointer(this->type), type_size(this->type));
            struct IRNode *value_store = ir_build_store(builder, value_alloca, value, type_size(this->type));
            variable_info->value = value_alloca;
            value_alloca->spill = true;
            value = value_alloca;
        }
    }
    else {
        if (this->addressed || ir_value_is_complex(this->type)) {
            value = ir_build_alloca(builder, type_pointer(this->type), type_size(this->type));
            value->spill = true;
        }
        else {
            value = ir_build_const(builder, this->type, 8, 0);
        }
    }
    variable_info->value = value;
    vpush(&builder->current_block->variable_list, variable_info);
}

void ir_build_type_definition(struct IRBuilder *builder, struct Node *node, struct TypeDefinition *this) {

}

void ir_build_return(struct IRBuilder *builder, struct Node *node, struct Return *this) {
    struct IRNode *value = ir_build(builder, this->expression);

    struct IRBlock *block = NULL;
    struct Vector *phi_values = NULL;
    struct Vector *phi_blocks = NULL;
    int sz = vsize(&builder->irblock_label_stack);
    for (int i = sz - 1; i >= 0; i--) {
        const char *label = builder->irblock_label_stack.ptr[i];
        if (!this->label || label && !_strcmp(this->label, label)) {
            block = builder->irblock_block_stack.ptr[i];
            phi_values = builder->irblock_phi_stack_values.ptr[i];
            phi_blocks = builder->irblock_phi_stack_blocks.ptr[i];
            break;
        }
    }
    if (!block) {
        _panic("Block not found in NodeReturn");
    }
    struct IRNode *value_br = ir_build_br(builder, block);
    if (value) {
        vpush(phi_values, value);
        vpush(phi_blocks, builder->current_block);
    }

    ir_create_edge(builder->current_block, block);
}

void ir_build_break(struct IRBuilder *builder, struct Node *node, struct Break *this) {
    struct IRNode *value = ir_build(builder, this->expression);

    struct IRBlock *block = NULL;
    struct Vector *phi_values = NULL;
    struct Vector *phi_blocks = NULL;
    int sz = vsize(&builder->irloop_label_stack);
    for (int i = sz - 1; i >= 0; i--) {
        const char *label = builder->irloop_label_stack.ptr[i];
        if (!this->label || label && !_strcmp(this->label, label)) {
            block = builder->irloop_blockend_stack.ptr[i];
            phi_values = builder->irloop_phi_stack_values.ptr[i];
            phi_blocks = builder->irloop_phi_stack_blocks.ptr[i];
            break;
        }
    }
    if (!block) {
        _panic("Block not found in NodeBreak");
    }
    struct IRNode *value_br = ir_build_br(builder, block);
    vpush(phi_values, value);
    vpush(phi_blocks, builder->current_block);

    ir_create_edge(builder->current_block, block);
}

void ir_build_continue(struct IRBuilder *builder, struct Node *node, struct Continue *this) {
    struct IRBlock *block = NULL;
    int sz = vsize(&builder->irloop_label_stack);
    for (int i = sz - 1; i >= 0; i--) {
        const char *label = builder->irloop_label_stack.ptr[i];
        if (!this->label || label && !_strcmp(this->label, label)) {
            block = builder->irloop_blockcond_stack.ptr[i];
            break;
        }
    }
    if (!block) {
        _panic("Block not found in NodeContinue");
    }
    struct IRNode *value_br = ir_build_br(builder, block);

    ir_create_edge(builder->current_block, block);

    int sz_var = vsize(&block->variable_list);
    int cnt = 0;
    for (int i = 0; i < sz_var; i++) {
        if (((struct IRVariableInfo*)block->variable_list.ptr[i])->addressed) continue;
        _assert(!_strcmp(
            ((struct IRVariableInfo*)block->variable_list.ptr[i])->name, 
            ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->name));
        struct IRPhi *tmp_value_phi = ((struct IRNode*)block->value_list.ptr[cnt])->node_ptr;
        cnt++;
        vpush(&tmp_value_phi->values, ((struct IRVariableInfo*)builder->current_block->variable_list.ptr[i])->value);
        vpush(&tmp_value_phi->blocks, builder->current_block);
    }
}

struct IRNode *ir_build_as(struct IRBuilder *builder, struct Node *node, struct As *this) {
    struct IRNode *value = ir_build(builder, this->expression);
    value->type = this->type;
    return value;
}

void ir_build_assignment(struct IRBuilder *builder, struct Node *node, struct Assignment *this) {
    struct Identifier *_identifier = (struct Identifier*)this->dst->node_ptr;
    
    struct IRNode *value = ir_build(builder, this->src);
    struct IRBlock *block = builder->current_block;
    int sz = vsize(&block->variable_list);
    for (int i = sz - 1; i >= 0; i--) {
        struct IRVariableInfo *variable_info = block->variable_list.ptr[i];
        if (!_strcmp(variable_info->name, _identifier->identifier)) {
            struct IRNode *value_to = variable_info->value;
            if (value_to->spill || ir_value_is_complex(this->src->type)) {
                struct IRNode *value_store = ir_build_store(builder, value_to, value, type_size(this->src->type));
            }
            else {
                variable_info->value = value;
            }
            return;
        }
    }
    
    sz = vsize(&builder->globalvar_list);
    for (int i = 0; i < sz; i++) {
        struct IRGlobalVar *globalvar = builder->globalvar_list.ptr[i];
        if (!_strcmp(globalvar->name, _identifier->identifier)) {
            ir_build_store(builder, globalvar->ir_value, value, type_size(this->src->type));
            return;
        }
    }

    _panic("Identifier not found in NodeAssignment");
}

void ir_build_movement(struct IRBuilder *builder, struct Node *node, struct Movement *this) {
    struct IRNode *value_dst = ir_build(builder, this->dst);
    struct IRNode *value_src = ir_build(builder, this->src);
    struct IRNode *value_store = ir_build_store(builder, value_dst, value_src, type_size(this->src->type));
}

struct IRNode *ir_build_identifier(struct IRBuilder *builder, struct Node *node, struct Identifier *this) {
    struct IRBlock *block = builder->current_block;
    
    struct IRNode *value = NULL;
    
    int sz = vsize(&builder->function_stack);
    for (int i = sz - 1; i >= 0; i--) {
        struct IRFunction *function = builder->function_stack.ptr[i];
        if (!_strcmp(this->identifier, function->name)) {
            value = function->ir_value;
            break;
        }
    }

    if (!value) {
        int sz = vsize(&block->variable_list);
        for (int i = sz - 1; i >= 0; i--) {
            struct IRVariableInfo *variable_info = block->variable_list.ptr[i];
            if (!_strcmp(this->identifier, variable_info->name)) {
                if (variable_info->addressed && !this->address && !ir_value_is_complex(node->type)) {
                    struct IRNode *value_load = ir_build_load(builder, type_deref(variable_info->value->type), variable_info->value, type_size(node->type));
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
            if (!_strcmp(this->identifier, globalvar->name)) {
                value = globalvar->ir_value;
                if (this->address) {
                    return value;
                }
                else {
                    struct IRNode *value_address = ir_build_load(builder, node->type, value, type_size(node->type));
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

struct IRNode *ir_build_integer(struct IRBuilder *builder, struct Node *node, struct Integer *this) {
    struct IRNode *value = ir_build_const(builder, node->type, 8, this->value);
    return value;
}

struct IRNode *ir_build_char(struct IRBuilder *builder, struct Node *node, struct Char *this) {
    struct IRNode *value = ir_build_const(builder, node->type, 1, this->value);
    return value;
}

struct IRNode *ir_build_string(struct IRBuilder *builder, struct Node *node, struct String *this) {
    const char *identifier = _concat("_L", _itoa(vsize(&builder->globalvar_list)));
    struct IRNode *value_global = ir_build_global(builder, node->type, identifier);
    vpush(&builder->globalvar_list, ir_build_globalvar(builder, identifier, IRGlobalVarString, (void*)this->value, value_global));
    return value_global;
}

struct IRNode *ir_build_array(struct IRBuilder *builder, struct Node *node, struct Array *this) {
    struct IRNode *value_alloca = ir_build_alloca(builder, type_pointer(node->type), type_size(node->type));

    struct TypeArray *type_array = (struct TypeArray*)value_alloca->type->node_ptr;
    int phase = 0;
    for (int i = 0; i < type_array->size; i++) {
        struct TypeNode *gep_type = type_copy_node(type_array->type);
        gep_type->degree++;

        struct IRNode *value = ir_build(builder, (struct Node*)this->values.ptr[i]);
        struct IRNode *value_gep = ir_build_gep(builder, gep_type, value_alloca, 
            ir_build_const(builder, create_type_int(0), 8, i), 
            type_size(type_array->type));
        struct IRNode *value_store = ir_build_store(builder, value_gep, value, type_size(type_array->type));

        phase += type_size(type_array->type);
    }

    return value_alloca;
}

struct IRNode *ir_build_struct_instance(struct IRBuilder *builder, struct Node *node, struct StructInstance *this) {
    struct IRNode *value_alloca = ir_build_alloca(builder, type_pointer(node->type), type_size(node->type));

    struct TypeStruct *type_struct = (struct TypeStruct*)value_alloca->type->node_ptr;
    int sz = vsize(&type_struct->types);
    int phase = 0;
    for (int i = 0; i < sz; i++) {
        struct TypeNode *type = (struct TypeNode*)type_struct->types.ptr[i];
        struct TypeNode *sgep_type = type_copy_node(type);
        sgep_type->degree++;

        struct IRNode *value = ir_build(builder, (struct Node*)this->values.ptr[i]);
        struct IRNode *value_sgep = ir_build_sgep(builder, sgep_type, value_alloca, phase);
        struct IRNode *value_store = ir_build_store(builder, value_sgep, value, type_size(type));

        phase += type_size(type);
    }

    return value_alloca;
}

struct IRNode *ir_build_lambda_function(struct IRBuilder *builder, struct Node *node, struct LambdaFunction *this) {
    struct FunctionSignature *signature = this->signature;
    struct IRFunction *function = ir_create_function(builder);
    function->type = node->type;
    function->function_definition = node;

    function->name = _concat("_", _itoa(builder->current_identifier));
    function->name_generate = function->name;
    builder->current_identifier++;

    struct IRNode *value = ir_build_global(builder, node->type, function->name_generate);
    function->ir_value = value;
    
    struct IRBlock *block;
    struct IRFunction *old_current_function = builder->current_function;
    struct IRBlock *old_current_block = builder->current_block;

    builder->current_function = function;
    block = ir_create_block(builder);
    builder->current_block = block;

    int sz = vsize(&signature->identifiers);
    for (int i = 0; i < sz; i++) {
        struct IRNode *value = ir_build_arg(builder, signature->types.ptr[i]);
        vpush(&function->arg_list, value);
        if (!builder->header) {
            Alloc(IRVariableInfo, variable_info);
            bool addressed = ((bool*)signature->addressed.ptr)[i];
            variable_info->name = signature->identifiers.ptr[i];
            variable_info->addressed = addressed;
            if (addressed) {
                struct IRNode *value_alloca = ir_build_alloca(builder, type_pointer(node->type), type_size(value->type));
                struct IRNode *value_store = ir_build_store(builder, value_alloca, value, type_size(value->type));
                variable_info->value = value_alloca;
                value_alloca->spill = true;
            }
            else {
                variable_info->value = value;
            }
            vpush(&block->variable_list, variable_info);
        }
    }

    struct IRNode *body_value = ir_build(builder, this->block);
    struct IRNode *ret_value = ir_build_ret(builder, body_value);

    builder->current_function = old_current_function;
    builder->current_block = old_current_block;

    return value;
}

struct IRNode *ir_build_sizeof(struct IRBuilder *builder, struct Node *node, struct Sizeof *this) {
    struct IRNode *value = ir_build_const(builder, node->type, 8, type_size(this->type));
    return value;
}

struct IRNode *ir_build_function_call(struct IRBuilder *builder, struct Node *node, struct FunctionCall *this) {
    struct IRNode *value_function = ir_build(builder, this->function);
    struct Vector arguments = vnew();
    int sz = vsize(&this->arguments);
    for (int i = 0; i < sz; i++) {
        vpush(&arguments, ir_build(builder, this->arguments.ptr[i]));
    }

    struct IRNode *call = ir_build_call(builder, node->type, value_function, arguments);
    return call;
}

struct IRNode *ir_build_method_call(struct IRBuilder *builder, struct Node *node, struct MethodCall *this) {
    struct IRNode *value_function = NULL;
    bool found = false;
    int sz = vsize(&builder->function_list);
    for (int i = 0; i < sz; i++) {
        struct IRFunction *function = builder->function_list.ptr[i];
        if (function->function_definition == this->function_definition) {
            value_function = function->ir_value;
            break;
        }
    }
    _assert(value_function != NULL);
    struct Vector arguments = vnew();
    sz = vsize(&this->arguments);
    vpush(&arguments, ir_build(builder, this->caller));
    for (int i = 0; i < sz; i++) {
        vpush(&arguments, ir_build(builder, this->arguments.ptr[i]));
    }

    struct IRNode *call = ir_build_call(builder, node->type, value_function, arguments);
    return call;
}

struct IRNode *ir_build_dereference(struct IRBuilder *builder, struct Node *node, struct Dereference *this) {
    struct IRNode *value = ir_build(builder, this->expression);
    struct IRNode *value_load = ir_build_load(builder, type_deref(node->type), value, type_size(this->expression->type));
    return value_load;
}

struct IRNode *ir_build_index(struct IRBuilder *builder, struct Node *node, struct Index *this) {
    int size;
    if (this->left->type->degree > 1) size = 8;
    else if(this->left->type->degree == 1) size = this->left->type->size;
    else _panic("Non pointer argument in GEP");
    
    struct IRNode *value_base = ir_build(builder, this->left);
    struct IRNode *value_index = ir_build(builder, this->right);
    struct IRNode *value_gep = ir_build_gep(builder, this->left->type, value_base, value_index, size);
    if (this->address) {
        return value_gep;
    }

    struct IRNode *value_load = ir_build_load(builder, node->type, value_gep, size);
    return value_load;
}

struct IRNode *ir_build_get_field(struct IRBuilder *builder, struct Node *node, struct GetField *this) {
    struct IRNode *value = ir_build(builder, this->left);

    struct IRNode *value_sgep = ir_build_sgep(builder, node->type, value, this->phase);
    if (ir_value_is_complex(node->type)) {
        value_sgep->type = type_copy_node(value_sgep->type);
        value_sgep->type->degree++;
        return value_sgep;
    }
    if (this->address) {
        return value_sgep;
    }
    
    value_sgep->type = type_copy_node(value_sgep->type);
    value_sgep->type->degree++;
    
    struct IRNode *value_load = ir_build_load(builder, node->type, value_sgep, type_size(node->type));
    return value_load;
}

struct IRNode *ir_build_arithmetic(struct IRBuilder *builder, struct Node *node, struct BinaryOperator *this) {
    struct IRNode *value1 = NULL;
    if (this->left) {
        value1 = ir_build(builder, this->left);
        if (value1->spill) {
            struct IRNode *value_load = ir_build_load(builder, this->left->type, value1, type_size(this->left->type));
            value1 = value_load;
        }
    }

    struct IRNode *value2 = ir_build(builder, this->right);
    if (value2->spill) {
        struct IRNode *value_load = ir_build_load(builder, this->right->type, value2, type_size(this->right->type));
        value2 = value_load;
    }

    struct IRNode *value = ir_build_binary_operator(builder, node->node_type - NodeAnd + IRNodeAnd, node->type, value1, value2);
    return value;
}

struct IRNode *ir_build(struct IRBuilder *builder, struct Node *node) {
    switch (node->node_type) {
        case NodeModule: ir_build_module(builder, node, (struct Module*)node->node_ptr); return NULL;
        case NodeBlock: return ir_build_block(builder, node, (struct Block*)node->node_ptr);
        case NodeInclude: ir_build_include(builder, node, (struct Include*)node->node_ptr); return NULL;
        case NodeTest: ir_build_test(builder, node, (struct Test*)node->node_ptr); return NULL;
        case NodeIf: return ir_build_if(builder, node, (struct If*)node->node_ptr);
        case NodeWhile: return ir_build_while(builder, node, (struct While*)node->node_ptr);
        case NodeFunctionDefinition: ir_build_function_definition(builder, node, (struct FunctionDefinition*)node->node_ptr); return NULL;
        case NodePrototype: ir_build_prototype(builder, node, (struct Prototype*)node->node_ptr); return NULL;
        case NodeGlobalDefinition: ir_build_global_definition(builder, node, (struct GlobalDefinition*)node->node_ptr); return NULL;
        case NodeDefinition: ir_build_definition(builder, node, (struct Definition*)node->node_ptr); return NULL;
        case NodeTypeDefinition: ir_build_type_definition(builder, node, (struct TypeDefinition*)node->node_ptr); return NULL;
        case NodeReturn: ir_build_return(builder, node, (struct Return*)node->node_ptr); return NULL;
        case NodeBreak: ir_build_break(builder, node, (struct Break*)node->node_ptr); return NULL;
        case NodeContinue: ir_build_continue(builder, node, (struct Continue*)node->node_ptr); return NULL;
        case NodeAs: return ir_build_as(builder, node, (struct As*)node->node_ptr);
        case NodeAssignment: ir_build_assignment(builder, node, (struct Assignment*)node->node_ptr); return NULL;
        case NodeMovement: ir_build_movement(builder, node, (struct Movement*)node->node_ptr); return NULL;
        case NodeIdentifier: return ir_build_identifier(builder, node, (struct Identifier*)node->node_ptr);
        case NodeInteger: return ir_build_integer(builder, node, (struct Integer*)node->node_ptr);
        case NodeChar: return ir_build_char(builder, node, (struct Char*)node->node_ptr);
        case NodeString: return ir_build_string(builder, node, (struct String*)node->node_ptr);
        case NodeArray: return ir_build_array(builder, node, (struct Array*)node->node_ptr);
        case NodeStructInstance: return ir_build_struct_instance(builder, node, (struct StructInstance*)node->node_ptr);
        case NodeLambdaFunction: return ir_build_lambda_function(builder, node, (struct LambdaFunction*)node->node_ptr);
        case NodeSizeof: return ir_build_sizeof(builder, node, (struct Sizeof*)node->node_ptr);
        case NodeFunctionCall: return ir_build_function_call(builder, node, (struct FunctionCall*)node->node_ptr);
        case NodeMethodCall: return ir_build_method_call(builder, node, (struct MethodCall*)node->node_ptr);
        case NodeDereference: return ir_build_dereference(builder, node, (struct Dereference*)node->node_ptr);
        case NodeIndex: return ir_build_index(builder, node, (struct Index*)node->node_ptr);
        case NodeGetField: return ir_build_get_field(builder, node, (struct GetField*)node->node_ptr);
        default: return ir_build_arithmetic(builder, node, (struct BinaryOperator*)node->node_ptr);
    }
}
