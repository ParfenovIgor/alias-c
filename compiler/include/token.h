#pragma once

typedef int TokenType;

enum TokenType {
    TokenInclude,
    TokenIf,
    TokenElse,
    TokenWhile,
    TokenFunc,
    TokenProto,
    TokenStruct,
    TokenDef,
    TokenTypedef,
    TokenReturn,
    TokenAs,
    TokenConst,
    TokenTest,
    TokenAssign,
    TokenMove,
    TokenDot,
    TokenComma,
    TokenColon,
    TokenSemicolon,
    TokenBraceOpen,
    TokenBraceClose,
    TokenParenthesisOpen,
    TokenParenthesisClose,
    TokenBracketOpen,
    TokenBracketClose,
    TokenBackslash,
    TokenPipe,
    TokenAddress,
    TokenDereference,
    TokenIndex,
    TokenGetField,
    TokenCaret,
    TokenSharp,
    TokenPlus,
    TokenMinus,
    TokenMult,
    TokenDiv,
    TokenLess,
    TokenGreater,
    TokenEqual,
    TokenInteger,
    TokenChar,
    TokenString,
    TokenIdentifier,
    TokenEof,
};

struct Token {
    TokenType type;
    int value_int;
    const char *value_string;
    int line_begin, position_begin, line_end, position_end;
    const char *filename;
};

struct TokenStream {
    struct Token *stream;
    int size;
    int reserved;
    int pos;
};

struct TokenStream *tokenstream_new();
void tokenstream_push(struct TokenStream*, struct Token);
struct Token tokenstream_get(struct TokenStream*);
struct Token tokenstream_get_prev(struct TokenStream*);
void tokenstream_next(struct TokenStream*);

struct Token token_build(TokenType type, int line_begin, int position_begin, int line_end, int position_end, const char *filename);
