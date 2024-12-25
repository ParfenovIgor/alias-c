#pragma once

#include <typeast.h>
#include <context.h>

bool type_equal(struct TypeNode*, struct TypeNode*, struct CPContext*);
struct TypeNode *type_get_struct_pointer(struct TypeNode*, struct CPContext*);
struct TypeNode *type_get_function(struct TypeNode*, struct CPContext*);
int type_size(struct TypeNode*, struct CPContext*);
struct TypeNode *type_copy_node(struct TypeNode*);
const char *type_mangle(struct TypeNode*, struct CPContext*);
