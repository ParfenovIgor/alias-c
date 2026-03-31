#pragma once

#include <ir.h>

struct IRBuilder {
    struct Vector function_list;
    struct Vector function_stack;
    struct Vector globalvar_list;
    struct IRFunction *current_function;
    struct IRBlock *current_block;

    bool header;
    bool testing;
    int current_identifier;
    struct Vector test_names;

    struct Vector nodes;
    struct Vector irblock_label_stack;
    struct Vector irblock_block_stack;
    struct Vector irblock_phi_stack;
    struct Vector irloop_label_stack;
    struct Vector irloop_blockcond_stack;
    struct Vector irloop_blockend_stack;
    struct Vector irloop_phi_stack;
};

struct IRBuilder *ir_builder(bool testing);
struct IRValue *ir_build_value(struct IRBuilder *builder, enum IRValueType value_type);
struct IRValue *ir_build_value_free(struct IRBuilder *builder, enum IRValueType value_type);
struct IRBlock *ir_build_block(struct IRBuilder *builder);
struct IRFunction *ir_build_function(struct IRBuilder *builder);
void ir_build_edge(struct IRBlock *block_out, struct IRBlock *block_in);
void ir_build_phi(struct IRBuilder *builder);
struct IRValue *ir_build(struct IRBuilder *builder, struct Node *node);