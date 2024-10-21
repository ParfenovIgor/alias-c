#pragma once

#include <token.h>
#include <ast.h>

void error_lexer(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename);
void error_syntax(const char *value, struct Token token);
void error_semantic(const char *value, struct Node *node);
