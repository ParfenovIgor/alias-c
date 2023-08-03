#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include "../header/common.h"
#include "../header/token.h"
#include "../header/ast.h"

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename);
void SyntaxError(const char *value, struct Token token);

#endif // EXCEPTION_H_INCLUDED
