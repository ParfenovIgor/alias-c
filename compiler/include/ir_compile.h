#pragma once

#include <ir.h>
#include <ir_build.h>

void ir_compile_type(struct TypeNode *type, int fd_text, bool in_parenthesis);
void ir_compile_type_prefix(struct TypeNode *type, int fd_text);
void ir_compile_type_suffix(struct TypeNode *type, int fd_text);
void ir_compile_value(struct Vector *values_list, struct IRValue *value, int fd_text);
void ir_compile_block(struct Vector *blocks_list, struct IRBlock *block, int fd_text);
void ir_compile_phi(struct Vector *values_list, struct IRBlock *block, struct IRBlock *succ_block, int fd_text);
void ir_compile(struct IRBuilder *builder, const char *filename_compile_output);