#pragma once

#include "../include/common.h"
#include "../include/token.h"

struct TokenStream *Lexer_Process(const char *str, const char *filename);
