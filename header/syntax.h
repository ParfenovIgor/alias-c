#ifndef SYNTAX_H_INCLUDED
#define SYNTAX_H_INCLUDED

#include "token.h"
#include "ast.h"

struct Node *Syntax_ProcessBlock(struct TokenStream*, bool braces);
struct Node *Syntax_ProcessExpression(struct TokenStream*);
struct Node *Syntax_ProcessPrimary(struct TokenStream*);
struct FunctionSignature *Syntax_ProcessFunctionSignature(struct TokenStream *ts);
const char **Syntax_ProcessMetavariables(struct TokenStream *ts);
struct Node *Syntax_ProcessStatement(struct TokenStream*);
struct Node *Syntax_Process(struct TokenStream*);

#endif // SYNTAX_H_INCLUDED
