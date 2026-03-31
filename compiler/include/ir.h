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

struct IRNode {
    void *node_ptr;
    enum IRValueType node_type;
    struct TypeNode *type;
    bool spill;
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
