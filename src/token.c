#include "../include/token.h"

struct Token TokenStream_GetToken(struct TokenStream *this) {
    return this->stream[this->pos];
}

void TokenStream_NextToken(struct TokenStream *this) {
    this->pos++;
}

struct Token Token_Build(TokenType _type, int _line_begin, int _position_begin, int _line_end, int _position_end, const char *_filename) {
    struct Token token;
    token.type = _type;
    token.line_begin = _line_begin;
    token.position_begin = _position_begin;
    token.line_end = _line_end;
    token.position_end = _position_end;
    token.filename = _filename;
    return token;
}
