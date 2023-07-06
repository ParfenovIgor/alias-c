#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

#include "ast.h"
#include "settings.h"
#include <stdio.h>

void Compile(struct Node*, FILE *out, struct Settings *settings);

#endif // COMPILE_H_INCLUDED