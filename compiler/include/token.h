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
    TokenEval,
    TokenReturn,
    TokenBreak,
    TokenContinue,
    TokenAs,
    TokenConst,
    TokenTest,
    TokenAnd,
    TokenOr,
    TokenNot,
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
    TokenAmpersand,
    TokenPipe,
    TokenCaret,
    TokenTilde,
    TokenBitwiseLeft,
    TokenBitwiseRight,
    TokenDereference,
    TokenGetField,
    TokenSharp,
    TokenPlus,
    TokenMinus,
    TokenMult,
    TokenDiv,
    TokenMod,
    TokenLess,
    TokenGreater,
    TokenEqual,
    TokenLessEqual,
    TokenGreaterEqual,
    TokenNotEqual,
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
struct Token *tokenstream_pget(struct TokenStream*);
struct Token tokenstream_get_prev(struct TokenStream*);
void tokenstream_next(struct TokenStream*);

struct Token token_build(TokenType type, int line_begin, int position_begin, int line_end, int position_end, const char *filename);
