#include <lexer.h>
#include <exception.h>
#include <algorithm.h>
#include <stdlib.h>
#include <string.h>

bool is_operator(const char *str, const char *word, int i) {
    return _strncmp(str + i, word, _strlen(word)) == 0;
}

char *string_push_back(char *a, char x) {
    int sz = _strlen(a);
    char *b = (char*)_malloc(sz + 2);
    _strcpy(b, a);
    _free(a);
    b[sz] = x;
    b[sz + 1] = '\0';
    return b;
}

int get_int_value(const char *str, int l, int r) {
    int res = 0;
    int sign = 1;
    if (str[l] == '-') {
        sign = -sign;
        l++;
    }
    for (int i = l; i <= r; i++) {
        res = res * 10 + (int)(str[i] - '0');
    }
    res *= sign;
    return res;
}

bool check_char(char format, char *out) {
    if (format == '0') {
        *out = 0x0;
        return true;
    }
    if (format == 'a') {
        *out = 0x7;
        return true;
    }
    if (format == 'b') {
        *out = 0x8;
        return true;
    }
    if (format == 'e') {
        *out = 0x1B;
        return true;
    }
    if (format == 'f') {
        *out = 0xC;
        return true;
    }
    if (format == 'n') {
        *out = 0xA;
        return true;
    }
    if (format == 'r') {
        *out = 0xD;
        return true;
    }
    if (format == 't') {
        *out = 0x9;
        return true;
    }
    if (format == 'v') {
        *out = 0xB;
        return true;
    }
    if (format == '\\') {
        *out = 0x5C;
        return true;
    }
    if (format == '\'') {
        *out = 0x27;
        return true;
    }
    if (format == '\"') {
        *out = 0x22;
        return true;
    }
    if (format == '?') {
        *out = 0x3F;
        return true;
    }
    return false;
}

bool check_token(const char *str, int len, const char *word, int i, bool oper) {
    if (!oper && i + _strlen(word) < len && 
        (_isdigit(str[i + _strlen(word)]) || 
         _isalpha(str[i + _strlen(word)]))) return false;

    return _strncmp(str + i, word, _strlen(word)) == 0;
}

bool append_token(const char *str, int len, int *i, const char *token_str, TokenType token_type, bool oper, int line, int *position, const char *filename, struct TokenStream *token_stream) {
    if (!oper && *i + _strlen(token_str) < len && 
        (_isdigit(str[*i + _strlen(token_str)]) || 
         _isalpha(str[*i + _strlen(token_str)]))) return false;
    if (_strncmp(str + *i, token_str, _strlen(token_str)) == 0) {
        struct Token token = token_build(token_type, line, *position, line, *position + _strlen(token_str) - 1, filename);
        tokenstream_push(token_stream, token);
        *i += _strlen(token_str);
        *position += _strlen(token_str);
        return true;
    }
    else return false;
}

struct TokenStream *lexer_process(const char *str, const char *filename) {
    struct TokenStream *token_stream = tokenstream_new();
    int line = 0, position = 0;
    int N = _strlen(str);
    for (int i = 0; i < N;) {
        if (check_token(str, N, "//", i, true)) {
            i++;
            while(i < N && str[i] != '\n') i++;
            i = _min(i + 1, N);
            position = 0;
            line++;
        }
        else if (check_token(str, N, "/*", i, true)) {
            i += 2;
            position += 2;
            while(i + 2 <= N && (str[i] != '*' || str[i + 1] != '/')) {
                i++;
                position++;
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
            }
            position += 2;
            i = _min(i + 2, N);
        }
        else if (append_token(str, N, &i, "include", TokenInclude, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "if", TokenIf, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "else", TokenElse, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "while", TokenWhile, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "func", TokenFunc, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "proto", TokenProto, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "struct", TokenStruct, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "def", TokenDef, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "typedef", TokenTypedef, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "return", TokenReturn, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "as", TokenAs, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "const", TokenConst, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "test", TokenTest, false, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ":=", TokenAssign, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "<-", TokenMove, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ",", TokenComma, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ".", TokenDot, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ":", TokenColon, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ";", TokenSemicolon, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "{", TokenBraceOpen, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "}", TokenBraceClose, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "(", TokenParenthesisOpen, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ")", TokenParenthesisClose, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "[", TokenBracketOpen, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "]", TokenBracketClose, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "\\", TokenBackslash, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "|", TokenPipe, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "&", TokenAddress, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "$", TokenDereference, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "!!", TokenIndex, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "->", TokenGetField, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "^", TokenCaret, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "#", TokenSharp, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "+", TokenPlus, true, line, &position, filename, token_stream)) {}
        else if (check_token(str, N, "-", i, true)) {
            if (i + 2 <= N && _isdigit(str[i + 1]) && 
               (token_stream->size == 0 || 
                   (token_stream->stream[token_stream->size - 1].type != TokenInteger && 
                    token_stream->stream[token_stream->size - 1].type != TokenIdentifier))) {
                int l = i;
                i += 2;
                while (i + 1 <= N && _isdigit(str[i])) i++;
                int r = i - 1;
                struct Token token = token_build(TokenInteger, line, position, line, position + r - l, filename);
                token.value_int = get_int_value(str, l, r);
                tokenstream_push(token_stream, token);
                position += r - l + 1;
            }
            else append_token(str, N, &i, "-", TokenMinus, true, line, &position, filename, token_stream);
        }
        else if (append_token(str, N, &i, "*", TokenMult, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "/", TokenDiv, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "<", TokenLess, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, ">", TokenGreater, true, line, &position, filename, token_stream)) {}
        else if (append_token(str, N, &i, "=", TokenEqual, true, line, &position, filename, token_stream)) {}
        else if (_isdigit(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && _isdigit(str[i])) i++;
            int r = i - 1;
            struct Token token = token_build(TokenInteger, line, position, line, position + r - l, filename);
            token.value_int = get_int_value(str, l, r);
            tokenstream_push(token_stream, token);
            position += r - l + 1;
        }
        else if (check_token(str, N, "\'", i, true)) {
            i++;
            int line_begin = line;
            int position_begin = position;
            position++;

            char ch;
            if (str[i] == '\\' && i + 1 < N && check_char(str[i + 1], &ch)) {
                i += 2;
                position += 2;
            }
            else {
                ch = str[i];
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }

            if (!(i < N && str[i] == '\'')) {
                error_lexer("\' expected after char", line_begin, position_begin, line, position, filename);
            }
            struct Token token = token_build(TokenChar, line_begin, position_begin, line, position, filename);
            token.value_int = ch;
            tokenstream_push(token_stream, token);
            position++;
            i++;
        }
        else if (check_token(str, N, "\"", i, true)) {
            i++;
            int line_begin = line;
            int position_begin = position;
            position++;
            char *buffer = (char*)_malloc(1024);
            int buf_len = 0;
            while (i < N && str[i] != '\"') {
                if (str[i] == '\\' && i + 1 < N) {
                    char ch;
                    if (check_char(str[i + 1], &ch)) {
                        buffer[buf_len] = ch;
                        buf_len++;
                        i += 2;
                        position += 2;
                        continue;
                    }
                }
                buffer[buf_len] = str[i];
                buf_len++;
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == N) {
                error_lexer("\" expected after string", line_begin, position_begin, line, position, filename);
            }
            buffer[buf_len] = '\0';
            struct Token token = token_build(TokenString, line_begin, position_begin, line, position, filename);
            token.value_string = buffer;
            token.value_int = buf_len;
            tokenstream_push(token_stream, token);
            position++;
            i++;
        }
        else if (_isalpha(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && (_isalpha(str[i]) || _isdigit(str[i]))) i++;
            int r = i - 1;
            struct Token token = token_build(TokenIdentifier, line, position, line, position + r - l, filename);
            token.value_string = _strndup(str + l, r - l + 1);
            tokenstream_push(token_stream, token);
            position += r - l + 1;
        }
        else if (str[i] == ' ' || str[i] == '\t' || str[i] == '\r') {
            i++;
            position++;
        }
        else if (str[i] == '\n') {
            i++;
            position = 0;
            line++;
        }
        else {
            error_lexer("Unexpected symbol", line, position, line, position, filename);
        }
    }

    struct Token token = token_build(TokenEof, line, position, line, position, filename);
    tokenstream_push(token_stream, token);

    return token_stream;
}

enum Color {
    Color_Black,
    Color_White,
    Color_DarkGray,
    Color_Gray,
    Color_LightGray,
    Color_Red,
    Color_Green,
    Color_Blue,
    Color_Cyan,
    Color_Magenta,
    Color_Yellow,
    Color_DarkRed,
    Color_DarkGreen,
    Color_DarkBlue,
    Color_DarkCyan,
    Color_DarkMagenta,
    Color_DarkYellow,
};

const char *ColorToString(enum Color color) {
    const char *strings[] = {
        "#000000",
        "#ffffff",
        "#808080",
        "#a0a0a4",
        "#c0c0c0",
        "#ff0000",
        "#00ff00",
        "#0000ff",
        "#00ffff",
        "#ff00ff",
        "#ffff00",
        "#800000",
        "#008000",
        "#000080",
        "#008080",
        "#800080",
        "#808000",
    };
    return strings[color];
}

const char *TokenColor(enum TokenType type, 
    int *brace_level,
    int *parenthesis_level,
    int *bracket_level) {
    enum Color colors[] = {
        Color_DarkGreen,    // TokenInclude,
        Color_Blue,         // TokenIf,
        Color_Blue,         // TokenElse,
        Color_Blue,         // TokenWhile,
        Color_Blue,         // TokenFunc,
        Color_Blue,         // TokenProto,
        Color_Blue,         // TokenStruct,
        Color_Blue,         // TokenDef,
        Color_Blue,         // TokenTypedef,
        Color_Blue,         // TokenReturn,
        Color_Blue,         // TokenAs,
        Color_Blue,         // TokenConst,
        Color_Blue,         // TokenTest,
        Color_Black,        // TokenAssign,
        Color_Black,        // TokenMove,
        Color_Black,        // TokenDot,
        Color_Black,        // TokenComma,
        Color_Black,        // TokenColon,
        Color_Black,        // TokenSemicolon,
        Color_White,        // TokenBraceOpen,
        Color_White,        // TokenBraceClose,
        Color_White,        // TokenParenthesisOpen,
        Color_White,        // TokenParenthesisClose,
        Color_White,        // TokenBracketOpen,
        Color_White,        // TokenBracketClose,
        Color_Black,        // TokenBackslash,
        Color_Black,        // TokenPipe,
        Color_Black,        // TokenAddress,
        Color_Black,        // TokenDereference,
        Color_Black,        // TokenIndex,
        Color_Black,        // TokenGetField,
        Color_Black,        // TokenCaret,
        Color_Black,        // TokenSharp,
        Color_Black,        // TokenPlus,
        Color_Black,        // TokenMinus,
        Color_Black,        // TokenMult,
        Color_Black,        // TokenDiv,
        Color_Black,        // TokenLess,
        Color_Black,        // TokenGreater,
        Color_Black,        // TokenEqual,
        Color_DarkGreen,    // TokenInteger,
        Color_DarkCyan,     // TokenChar,
        Color_Red,          // TokenString,
        Color_DarkYellow,   // TokenIdentifier,
        Color_White,        // TokenEof,
    };
    if (type == TokenBraceOpen) {
        int id = 5 + *brace_level % 12;
        (*brace_level)++;
        return ColorToString(id);
    }
    if (type == TokenBraceClose) {
        (*brace_level)--;
        int id = 5 + *brace_level % 12;
        return ColorToString(id);
    }
    if (type == TokenParenthesisOpen) {
        int id = 5 + *parenthesis_level % 12;
        (*parenthesis_level)++;
        return ColorToString(id);
    }
    if (type == TokenParenthesisClose) {
        (*parenthesis_level)--;
        int id = 5 + *parenthesis_level % 12;
        return ColorToString(id);
    }
    if (type == TokenBracketOpen) {
        int id = 5 + *bracket_level % 12;
        (*bracket_level)++;
        return ColorToString(id);
    }
    if (type == TokenBracketClose) {
        (*bracket_level)--;
        int id = 5 + *bracket_level % 12;
        return ColorToString(id);
    }
    return ColorToString(colors[type]);
}

#include <stdio.h>
#include <sys/time.h>

const char *lexer_highlight(const char *str) {
    struct TokenStream *token_stream = lexer_process(str, "");

    int cnt_tokens, buffer_len = 0;
    for (cnt_tokens = 0; token_stream->stream[cnt_tokens].type != TokenEof; cnt_tokens++);
    char **buffers = _malloc(sizeof(char*) * cnt_tokens);
    int brace_level = 0;
    int parenthesis_level = 0;
    int bracket_level = 0;
    
    for (int i = 0; i < cnt_tokens; i++) {
        const int buffer_size = 128;
        buffers[i] = _malloc(sizeof(char) * buffer_size);
        char *str = buffers[i];
        int pos = 0;
        pos += _sputs(str + pos, "{\"lb\":");
        pos += _sputi(str + pos, token_stream->stream[i].line_begin);
        pos += _sputs(str + pos, ",\"pb\":");
        pos += _sputi(str + pos, token_stream->stream[i].position_begin);
        pos += _sputs(str + pos, ",\"le\":");
        pos += _sputi(str + pos, token_stream->stream[i].line_end);
        pos += _sputs(str + pos, ",\"pe\":");
        pos += _sputi(str + pos, token_stream->stream[i].position_end);
        pos += _sputs(str + pos, ",\"cl\":\"");
        pos += _sputs(str + pos,
            TokenColor(token_stream->stream[i].type,
            &brace_level,
            &parenthesis_level,
            &bracket_level));
        pos += _sputs(str + pos, "\"}");
        str[pos] = '\0';
        buffer_len += pos;
    }

    buffer_len += cnt_tokens + 3;
    char *buffer = _malloc(sizeof(char) * buffer_len);
    buffer[0] = '[';
    int pos = 1;
    for (int i = 0; i < cnt_tokens; i++) {
        _strcpy(buffer + pos, buffers[i]);
        pos += _strlen(buffers[i]);
        _free(buffers[i]);
        if (i + 1 < cnt_tokens) {
            buffer[pos] = ',';
            pos++;
        }
    }
    buffer[pos] = ']';
    pos++;
    buffer[pos] = '\0';

    return buffer;
}
