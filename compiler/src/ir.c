#include <ir.h>
#include <vector.h>
#include <stdlib.h>

struct IRBuilder *ir_builder(bool testing) {
    struct IRBuilder *builder = (struct IRBuilder*)_malloc(sizeof(struct IRBuilder));
    builder->current_function = NULL;
    builder->current_block = NULL;
    builder->function_list = vnew();
    builder->function_stack = vnew();
    builder->globalvar_list = vnew();
    builder->nodes = vnew();
    builder->irblock_label_stack = vnew();
    builder->irblock_block_stack = vnew();
    builder->irblock_phi_stack_values = vnew();
    builder->irblock_phi_stack_blocks = vnew();
    builder->irloop_label_stack = vnew();
    builder->irloop_blockend_stack = vnew();
    builder->irloop_blockcond_stack = vnew();
    builder->irloop_phi_stack_values = vnew();
    builder->irloop_phi_stack_blocks = vnew();
    builder->header = false;
    builder->testing = testing;
    builder->current_identifier = 0;
    builder->test_names = vnew();
    builder->value_idx = 0;
    builder->block_idx = 0;
    builder->loop_degree = 0;
    return builder;
}

bool ir_value_is_terminator(struct IRNode *value) {
    enum IRNodeType type = value->node_type;
    return type == IRNodeBr || type == IRNodeCondBr || type == IRNodeRet;
}

bool ir_value_has_value(struct IRNode *value) {
    enum IRNodeType type = value->node_type;
    return type != IRNodeStore && 
        type != IRNodeBr && type != IRNodeCondBr && type != IRNodeRet && 
        !(type == IRNodeCall && value->type->size == 0);
}

bool ir_value_is_complex(struct TypeNode *type) {
    return type->node_type == TypeNodeStruct && type->degree == 0;
}
