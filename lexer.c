#include "common.h"
#include "lexer.h"
#include "exception.h"
#include <stdlib.h>

int min(int a, int b) {
    if (a <= b) return a;
    else return b;
}

bool is_alpha(char c) {
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') || c == '_');
}

bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

bool is_reserved_word(const char *str, const char *word, int i) {
    int n = strlen(str);
    int m = strlen(word);
    if (i + m <= n) {
        for (int j = 0; j < m; j++) {
            if (str[i + j] != word[j]) {
                return false;
            }
        }
        if (i + m < n) {
            if (is_digit(str[i + m]) || is_alpha(str[i + m])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool is_operator(const char *str, const char *word, int i) {
    int n = strlen(str);
    int m = strlen(word);
    if (i + m <= n) {
        for (int j = 0; j < m; j++) {
            if (str[i + j] != word[j]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

char *string_push_back(char *a, char x) {
    int sz = strlen(a);
    char *b = (char*)malloc(sz + 2);
    strcpy(b, a);
    free(a);
    b[sz] = x;
    b[sz + 1] = '\0';
    return b;
}

void tokens_push_back(struct TokenStream *a, struct Token x) {
    struct Token *_stream = (struct Token*)malloc((a->stream_size + 1) * sizeof(struct Token));
    for (int i = 0; i < a->stream_size; i++) {
        _stream[i] = a->stream[i];
    }
    _stream[a->stream_size] = x;
    free(a->stream);
    a->stream = _stream;
    a->stream_size++;
}

int get_int_value(const char *str, int l, int r) {
    int res = 0;
    int sign = 1;
    if (str[l] == '-') {
        sign = -sign;
        l++;
    }
    for (int i = l; i <= r; i++) {
        res = res * 10 + (int)str[i];
    }
    res *= sign;
    return res;
}

struct TokenStream Lexer_Process(const char *str, const char *filename) {
    struct TokenStream token_stream;
    int line = 0, position = 0;
    int N = strlen(str);
    for (int i = 0; i < N;) {
        if (is_reserved_word(str, "asm", i)) {
            int line_begin = line;
            int position_begin = position;
            i += 3;
            position += 3;
            while (i < N && str[i] != '{') {
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                LexerError("{ expected after asm", line_begin, position_begin, line, position, filename);
            }
            position++;
            i++;
            char *code = (char*)malloc(1);
            code[0] = '\0';
            while (i < N && str[i] != '}') {
                code = string_push_back(code, str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                LexerError("} expected after asm", line_begin, position_begin, line, position, filename);
            }
            struct Token token = Token_Build(Asm, line_begin, position_begin, line, position, filename);
            token.value_string = code;
            tokens_push_back(&token_stream, token);
            position++;
            i++;
        }
        else if (is_reserved_word(str, "include", i)) {
            int line_begin = line;
            int position_begin = position;
            i += 7;
            position += 7;
            while (i < N && str[i] != '{') {
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                LexerError("{ expected after include", line_begin, position_begin, line, position, filename);
            }
            position++;
            i++;
            char *code = (char*)malloc(1);
            code[0] = '\0';
            while (i < N && str[i] != '}') {
                code = string_push_back(code, str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                LexerError("} expected after include", line_begin, position_begin, line, position, filename);
            }
            struct Token token = Token_Build(Include, line_begin, position_begin, line, position, filename);
            token.value_string = code;
            tokens_push_back(&token_stream, token);
            position++;
            i++;
        }
        else if (is_reserved_word(str, "int", i)) {
            struct Token token = Token_Build(Int, line, position, line, position + 2, filename);
            tokens_push_back(&token_stream, token);
            i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "ptr", i)) {
            struct Token token = Token_Build(Ptr, line, position, line, position + 2, filename);
            tokens_push_back(&token_stream, token);i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "if", i)) {
            struct Token token = Token_Build(If, line, position, line, position + 1, filename);
            tokens_push_back(&token_stream, token);
            i += 2;
            position += 2;
        }
        else if (is_reserved_word(str, "else", i)) {
            struct Token token = Token_Build(Else, line, position, line, position + 3, filename);
            tokens_push_back(&token_stream, token);
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "while", i)) {
            struct Token token = Token_Build(While, line, position, line, position + 4, filename);
            tokens_push_back(&token_stream, token);
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "func", i)) {
            struct Token token = Token_Build(Func, line, position, line, position + 3, filename);
            tokens_push_back(&token_stream, token);
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "proto", i)) {
            struct Token token = Token_Build(Proto, line, position, line, position + 4, filename);
            tokens_push_back(&token_stream, token);
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "def", i)) {
            struct Token token = Token_Build(Def, line, position, line, position + 2, filename);
            tokens_push_back(&token_stream, token);
            i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "const", i)) {
            struct Token token = Token_Build(Const, line, position, line, position + 4, filename);
            tokens_push_back(&token_stream, token);
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "assume", i)) {
            struct Token token = Token_Build(Assume, line, position, line, position + 5, filename);
            tokens_push_back(&token_stream, token);
            i += 6;
            position += 6;
        }
        else if (is_reserved_word(str, "alloc", i)) {
            struct Token token = Token_Build(Alloc, line, position, line, position + 4, filename);
            tokens_push_back(&token_stream, token);
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "free", i)) {
            struct Token token = Token_Build(Free, line, position, line, position + 3, filename);
            tokens_push_back(&token_stream, token);
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "call", i)) {
            struct Token token = Token_Build(Call, line, position, line, position + 3, filename);
            tokens_push_back(&token_stream, token);
            i += 4;
            position += 4;
        }
        else if (is_operator(str, ":=", i)) {
            struct Token token = Token_Build(Assign, line, position, line, position + 1, filename);
            tokens_push_back(&token_stream, token);
            i += 2;
            position += 2;
        }
        else if (is_operator(str, "<-", i)) {
            struct Token token = Token_Build(Move, line, position, line, position + 1, filename);
            tokens_push_back(&token_stream, token);
            i += 2;
            position += 2;
        }
        else if (is_operator(str, ",", i)) {
            struct Token token = Token_Build(Comma, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, ":", i)) {
            struct Token token = Token_Build(Colon, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, ";", i)) {
            struct Token token = Token_Build(Semicolon, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "{", i)) {
            struct Token token = Token_Build(BraceOpen, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "}", i)) {
            struct Token token = Token_Build(BraceClose, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "(", i)) {
            struct Token token = Token_Build(ParenthesisOpen, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, ")", i)) {
            struct Token token = Token_Build(ParenthesisClose, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "[", i)) {
            struct Token token = Token_Build(BracketOpen, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "]", i)) {
            struct Token token = Token_Build(BracketClose, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "$", i)) {
            struct Token token = Token_Build(Dereference, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "^", i)) {
            struct Token token = Token_Build(Caret, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "+", i)) {
            struct Token token = Token_Build(Plus, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "-", i)) {
            if (i + 2 <= N && is_digit(str[i + 1]) && 
               (token_stream.stream_size == 0 || 
                   (token_stream.stream[token_stream.stream_size - 1].type != Integer && 
                    token_stream.stream[token_stream.stream_size - 1].type != Identifier))) {
                int l = i;
                i += 2;
                while (i + 1 <= N && is_digit(str[i])) i++;
                int r = i - 1;
                struct Token token = Token_Build(Integer, line, position, line, position + r - l, filename);
                token.value_int = get_int_value(str, l, r);
                tokens_push_back(&token_stream, token);
                position += r - l + 1;
            }
            else {
                struct Token token = Token_Build(Minus, line, position, line, position, filename);
                tokens_push_back(&token_stream, token);
                i += 1;
                position += 1;
            }
        }
        else if (is_operator(str, "*", i)) {
            struct Token token = Token_Build(Mult, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "/", i) && (i + 2 > N || 
            (str[i + 1] != '/' && str[i + 1] != '*'))) {
            struct Token token = Token_Build(Div, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "<", i)) {
            struct Token token = Token_Build(Less, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "=", i)) {
            struct Token token = Token_Build(Equal, line, position, line, position, filename);
            tokens_push_back(&token_stream, token);
            i += 1;
            position += 1;
        }
        else if (is_operator(str, "//", i)) {
            i++;
            while(i < N && str[i] != '\n') i++;
            i = min(i + 1, N);
            position = 0;
            line++;
        }
        else if (is_operator(str, "/&", i)) {
            i += 2;
            position += 2;
            while(i + 2 <= N && (str[i] != '&' || str[i + 1] != '/')) {
                i++;
                position++;
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
            }
            position += 2;
            i = min(i + 2, N);
        }
        else if (is_operator(str, "\"", i)) {
            i++;
            int line_begin = line;
            int position_begin = position;
            position++;
            char *buffer = (char*)malloc(1);
            buffer[0] = '\0';
            while (i < N && str[i] != '\"') {
                if (str[i] == '\\' && i + 1 < N) {
                    bool found = false;
                    if (str[i + 1] == '0') {
                        buffer = string_push_back(buffer, (char)0x0);
                        found = true;
                    }
                    if (str[i + 1] == 'a') {
                        buffer = string_push_back(buffer, (char)0x7);
                        found = true;
                    }
                    if (str[i + 1] == 'b') {
                        buffer = string_push_back(buffer, (char)0x8);
                        found = true;
                    }
                    if (str[i + 1] == 'e') {
                        buffer = string_push_back(buffer, (char)0x1B);
                        found = true;
                    }
                    if (str[i + 1] == 'f') {
                        buffer = string_push_back(buffer, (char)0xC);
                        found = true;
                    }
                    if (str[i + 1] == 'n') {
                        buffer = string_push_back(buffer, (char)0xA);
                        found = true;
                    }
                    if (str[i + 1] == 'r') {
                        buffer = string_push_back(buffer, (char)0xD);
                        found = true;
                    }
                    if (str[i + 1] == 't') {
                        buffer = string_push_back(buffer, (char)0x9);
                        found = true;
                    }
                    if (str[i + 1] == 'v') {
                        buffer = string_push_back(buffer, (char)0xB);
                        found = true;
                    }
                    if (str[i + 1] == '\\') {
                        buffer = string_push_back(buffer, (char)0x5C);
                        found = true;
                    }
                    if (str[i + 1] == '\'') {
                        buffer = string_push_back(buffer, (char)0x27);
                        found = true;
                    }
                    if (str[i + 1] == '\"') {
                        buffer = string_push_back(buffer, (char)0x22);
                        found = true;
                    }
                    if (str[i + 1] == '?') {
                        buffer = string_push_back(buffer, (char)0x3F);
                        found = true;
                    }
                    if (found) {
                        i += 2;
                        position += 2;
                        continue;
                    }
                }
                buffer = string_push_back(buffer, str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                LexerError("\" expected after string", line_begin, position_begin, line, position, filename);
            }
            buffer = string_push_back(buffer, (char)0x0);
            struct Token token = Token_Build(String, line_begin, position_begin, line, position, filename);
            token.value_string = buffer;
            tokens_push_back(&token_stream, token);
            position++;
            i++;
        }
        else if (is_digit(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && is_digit(str[i])) i++;
            int r = i - 1;
            struct Token token = Token_Build(Integer, line, position, line, position + r - l, filename);
            token.value_int = get_int_value(str, l, r);
            tokens_push_back(&token_stream, token);
            position += r - l + 1;
        }
        else if (is_alpha(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && (is_alpha(str[i]) || is_digit(str[i]))) i++;
            int r = i - 1;
            struct Token token = Token_Build(Identifier, line, position, line, position + r - l, filename);
            token.value_string = _strndup(str + l, r - l + 1);
            tokens_push_back(&token_stream, token);
            position += r - l + 1;
        }
        else if (str[i] == ' ' || str[i] == '\t') {
            i++;
            position++;
        }
        else if (str[i] == '\n') {
            i++;
            position = 0;
            line++;
        }
        else {
            LexerError("Unexpected symbol", line, position, line, position, filename);
        }
    }

    struct Token token = Token_Build(Eof, line, position, line, position, filename);
    tokens_push_back(&token_stream, token);

    return token_stream;
}