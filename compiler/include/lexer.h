#pragma once

#include <token.h>

struct TokenStream *lexer_process(const char *str, const char *filename);
const char *lexer_highlight(const char *str);
