#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "common.h"
#include "token.h"

struct TokenStream *Lexer_Process(const char *str, const char *filename);

#endif // LEXER_H_INCLUDED
