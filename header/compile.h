#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

#include "../header/ast.h"
#include "../header/settings.h"
#include <stdio.h>

void Compile(struct Node*, FILE *out, struct Settings *settings);

#endif // COMPILE_H_INCLUDED