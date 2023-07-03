#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include "common.h"
#include "token.h"
#include "ast.h"

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename);
void SyntaxError(const char *value, struct Token token);

#endif // EXCEPTION_H_INCLUDED
