#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "../header/common.h"
#include "../header/ast.h"

void **push_back(void **a, void *x);
void **pop_back(void **a);
void *get_back(void **a);
int get_size(void **a);

#endif // VECTOR_H_INCLUDED