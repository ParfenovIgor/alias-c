#pragma once

#include <typeast.h>
#include <context.h>

struct TypeNode *type_normalize(struct TypeNode*, struct CPContext*);
bool type_equal(struct TypeNode*, struct TypeNode*, struct CPContext*);
struct TypeNode *type_get_struct_pointer(struct TypeNode*, struct CPContext*);
struct TypeNode *type_get_function(struct TypeNode*, struct CPContext*);
int type_size(struct TypeNode*);
struct TypeNode *type_copy_node(struct TypeNode*);
struct TypeNode *type_pointer(struct TypeNode*);
struct TypeNode *type_deref(struct TypeNode*);
const char *type_mangle(struct TypeNode*, struct CPContext*);
