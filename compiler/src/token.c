#include <token.h>
#include <stdlib.h>
#include <memory.h>
#include <cassert.h>

struct TokenStream *tokenstream_new() {
    struct TokenStream *ts = (struct TokenStream*)_malloc(sizeof(struct TokenStream));
    ts->stream = _malloc(sizeof(struct Token));
    ts->size = 0;
    ts->reserved = 1;
    ts->pos = 0;
    return ts;
}

void tokenstream_push(struct TokenStream *this, struct Token token) {
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

struct Token tokenstream_get(struct TokenStream *this) {
    return this->stream[this->pos];
}

struct Token *tokenstream_pget(struct TokenStream *this) {
    return &this->stream[this->pos];
}

struct Token tokenstream_get_prev(struct TokenStream *this) {
    _assert(this->pos > 0);
    return this->stream[this->pos - 1];
}

void tokenstream_next(struct TokenStream *this) {
    this->pos++;
}

struct Token token_build(TokenType type, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    struct Token token;
    token.type = type;
    token.line_begin = line_begin;
    token.position_begin = position_begin;
    token.line_end = line_end;
    token.position_end = position_end;
    token.filename = filename;
    return token;
}
