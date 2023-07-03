#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include "ast.h"

struct Node *Parse(const char *filename);
int Process();

#endif // PROCESS_H_INCLUDED