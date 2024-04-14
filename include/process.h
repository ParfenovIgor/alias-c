#pragma once

#include "../include/ast.h"
#include "../include/process.h"
#include "../include/settings.h"

struct Node *Parse(const char *filename);
int Process(struct Settings *settings);
