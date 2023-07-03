#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

typedef int TokenType;

enum TokenType {
    TokenIdentifier,
    TokenBraceOpen,
    TokenBraceClose,
    TokenParenthesisOpen,
    TokenParenthesisClose,
    TokenBracketOpen,
    TokenBracketClose,
    TokenAsm,
    TokenInclude,
    TokenComma,
    TokenColon,
    TokenSemicolon,
    TokenInt,
    TokenPtr,
    TokenIf,
    TokenElse,
    TokenWhile,
    TokenFunc,
    TokenProto,
    TokenDef,
    TokenConst,
    TokenAssign,
    TokenMove,
    TokenAssume,
    TokenInteger,
    TokenString,
    TokenDereference,
    TokenCaret,
    TokenPlus,
    TokenMinus,
    TokenMult,
    TokenDiv,
    TokenLess,
    TokenEqual,
    TokenAlloc,
    TokenFree,
    TokenCall,
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
    int stream_size;
    int reserved_size;
    int pos;
};

struct Token TokenStream_GetToken(struct TokenStream *this);

void TokenStream_NextToken(struct TokenStream *this);

struct Token Token_Build(TokenType _type, int _line_begin, int _position_begin, int _line_end, int _position_end, const char *_filename);

#endif // TOKEN_H_INCLUDED
