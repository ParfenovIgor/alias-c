#pragma once

#include <typeast.h>
#include <context.h>

bool type_equal(struct TypeNode*, struct TypeNode*);
int type_size(struct TypeNode*, struct CPContext*);
