#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include "common.h"
#include "ast.h"

void **push_back(void **a, void *x);
void **pop_back(void **a);
void *get_back(void **a);
int get_size(void **a);

#endif // VECTOR_H_INCLUDED