#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "../header/common.h"
#include "../header/token.h"

struct TokenStream *Lexer_Process(const char *str, const char *filename);

#endif // LEXER_H_INCLUDED
