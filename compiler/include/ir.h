#pragma once

#include <vector.h>
#include <type.h>
#include <typeast.h>
#include <cassert.h>
#include <stdbool.h>
#include <panic.h>

enum IRNodeType {
    IRNodeArg,
    IRNodeConst,
    IRNodeGlobal,
    IRNodePhi,
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

enum Register {
    REGNONE,
    RAX,
    RBX,
    RCX,
    RDX,
    RDI,
    RSI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
};

struct IRBlock;

struct IRNode {
    int idx;
    void *node_ptr;
    enum IRNodeType node_type;
    struct TypeNode *type;
    bool spill;
    int stack_phase;
    enum Register reg;
    struct Vector uses;
    struct Vector uses_address;
    struct IRBlock *block;
    int loop_degree;
    int right_bound;
};

struct IRArg {

};

struct IRConst {
    long size;
    long value;
};

struct IRGlobal {
    const char *name;
};

struct IRPhi {
    struct Vector values;
    struct Vector blocks;
};

struct IRGEP {
    struct IRNode *base;
    struct IRNode *index;
    long size;
};

struct IRSGEP {
    struct IRNode *instance;
    long phase;
};

struct IRAlloca {
    long size;
};

struct IRLoad {
    struct IRNode *src;
    long size;
};

struct IRStore {
    struct IRNode *dst;
    struct IRNode *src;
    long size;
};

struct IRCall {
    struct IRNode *function;
    struct Vector arguments;
};

struct IRBr {
    struct IRBlock *block;
};

struct IRCondBr {
    struct IRNode *condition;
    struct IRBlock *block_then;
    struct IRBlock *block_else;
};

struct IRRet {
    struct IRNode *value;
};

struct IRBinaryOperator {
    struct IRNode *left;
    struct IRNode *right;
};

struct IRVariableInfo {
    const char *name;
    struct IRNode *value;
    bool addressed;
};

struct IRBlock {
    int idx;
    struct Vector value_list;
    struct Vector succ_list;
    struct Vector pred_list;

    struct Vector variable_list;
};

struct IRFunction {
    const char *name;
    const char *name_generate;
    struct TypeNode *type;
    struct Vector arg_list;
    struct Vector block_list;
    struct IRNode *ir_value;
    struct Node *function_definition;
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
    struct IRNode *ir_value;
};

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

    int value_idx;
    int block_idx;
    int loop_degree;

    struct Vector nodes;
    struct Vector irblock_label_stack;
    struct Vector irblock_block_stack;
    struct Vector irblock_phi_stack_values;
    struct Vector irblock_phi_stack_blocks;
    struct Vector irloop_label_stack;
    struct Vector irloop_blockcond_stack;
    struct Vector irloop_blockend_stack;
    struct Vector irloop_phi_stack_values;
    struct Vector irloop_phi_stack_blocks;
};

struct IRBuilder *ir_builder(bool testing);
bool ir_value_is_terminator(struct IRNode *value);
bool ir_value_has_value(struct IRNode *value);
bool ir_value_is_complex(struct TypeNode *type);
