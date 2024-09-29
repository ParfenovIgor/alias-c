#include <token.h>
#include <stdlib.h>
#include <memory.h>

struct TokenStream *TokenStream_New() {
    struct TokenStream *ts = (struct TokenStream*)_malloc(sizeof(struct TokenStream));
    ts->stream = _malloc(sizeof(struct Token));
    ts->size = 0;
    ts->reserved = 1;
    ts->pos = 0;
    return ts;
}

void TokenStream_PushBack(struct TokenStream *this, struct Token token) {
    if (this->size + 1 > this->reserved) {
        int new_reserved = this->reserved * 2;
        struct Token *new_ptr = _malloc(new_reserved * sizeof(struct Token));
        _memcpy(new_ptr, this->stream, this->reserved * sizeof(struct Token));
        _free(this->stream);
        this->stream = new_ptr;
        this->reserved = new_reserved;
    }
    this->stream[this->size] = token;
    this->size++;
}

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
