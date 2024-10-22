#pragma once

#include <ast.h>
#include <process.h>
#include <settings.h>

struct Node *process_parse(const char*, struct Settings*);
int process(struct Settings*);
