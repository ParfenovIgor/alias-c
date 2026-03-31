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

struct IRVariableInfo {
    const char *name;
    struct IRValue *value;
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
    struct IRValue *ir_value;
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
    struct IRValue *ir_value;
};
