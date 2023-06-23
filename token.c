#include "token.h"

struct Token TokenStream_GetToken(struct TokenStream *this) {
    return this->stream[this->pos];
}

void TokenStream_NextToken(struct TokenStream *this) {
    this->pos++;
}