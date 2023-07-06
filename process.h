#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include "ast.h"
#include "process.h"
#include "settings.h"

struct Node *Parse(const char *filename);
int Process(struct Settings *settings);

#endif // PROCESS_H_INCLUDED