#ifndef SYNTAX_H_INCLUDED
#define SYNTAX_H_INCLUDED

#include "token.h"
#include "ast.h"

struct Node *Syntax_ProcessProgram(struct TokenStream*);
struct Node *Syntax_ProcessBlock(struct TokenStream*);
struct Node *Syntax_ProcessExpression(struct TokenStream*);
struct Node *Syntax_ProcessPrimary(struct TokenStream*);
struct Node *Syntax_ProcessStatement(struct TokenStream*);
struct Node *Syntax_Process(struct Token *token_stream, int token_stream_size) {return 0;}

#endif // SYNTAX_H_INCLUDED
