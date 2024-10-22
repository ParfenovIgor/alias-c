#pragma once

#include <token.h>
#include <ast.h>
#include <settings.h>

struct Node *syntax_process(struct TokenStream*, struct Settings*);
