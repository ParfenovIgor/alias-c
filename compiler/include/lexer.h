#pragma once

#include <common.h>
#include <token.h>

struct TokenStream *Lexer_Process(const char *str, const char *filename);
const char *Lexer_Highlight(const char *str);
