#pragma once

#include "../include/common.h"
#include "../include/ast.h"

void **push_back(void **a, void *x);
void **pop_back(void **a);
void *get_back(void **a);
int get_size(void **a);
