#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

#include "ast.h"
#include <stdio.h>

void Compile(struct Node*, FILE *out);

#endif // COMPILE_H_INCLUDED