#pragma once

typedef int TokenType;

enum TokenType {
    TokenAsm,
    TokenInclude,
    TokenIf,
    TokenElse,
    TokenWhile,
    TokenFunc,
    TokenProto,
    TokenStruct,
    TokenDef,
    TokenReturn,
    TokenAs,
    TokenConst,
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
    TokenAddress,
    TokenDereference,
    TokenIndex,
    TokenGetField,
    TokenCaret,
    TokenPlus,
    TokenMinus,
    TokenMult,
    TokenDiv,
    TokenLess,
    TokenGreater,
    TokenEqual,
    TokenString,
    TokenInteger,
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

struct TokenStream *TokenStream_New();
void TokenStream_PushBack(struct TokenStream *this, struct Token token);
struct Token TokenStream_GetToken(struct TokenStream *this);
void TokenStream_NextToken(struct TokenStream *this);

struct Token Token_Build(TokenType _type, int _line_begin, int _position_begin, int _line_end, int _position_end, const char *_filename);
