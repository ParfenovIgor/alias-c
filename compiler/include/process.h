#pragma once

#include <ast.h>
#include <process.h>
#include <settings.h>

struct Node *Parse(const char *filename);
int Process(struct Settings *settings);
