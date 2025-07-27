#pragma once

#include <ast.h>
#include <settings.h>

#define WORD 8

int align_to_word(int x);
void compile_process(struct Node*, struct Settings*);
