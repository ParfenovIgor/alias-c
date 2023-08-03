#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include "../header/ast.h"
#include "../header/process.h"
#include "../header/settings.h"

struct Node *Parse(const char *filename);
int Process(struct Settings *settings);

#endif // PROCESS_H_INCLUDED