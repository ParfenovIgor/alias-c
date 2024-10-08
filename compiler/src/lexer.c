#include <lexer.h>
#include <exception.h>
#include <algorithm.h>
#include <stdlib.h>
#include <string.h>

bool CheckToken(const char *str, int len, const char *word, int i, bool oper) {
    if (!oper && i + _strlen(word) < len && 
        (_isdigit(str[i + _strlen(word)]) || 
         _isalpha(str[i + _strlen(word)]))) return false;

    return _strncmp(str + i, word, _strlen(word)) == 0;
}

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

bool AppendToken(const char *str, int len, int *i, const char *token_str, TokenType token_type, bool oper, int line, int *position, const char *filename, struct TokenStream *token_stream) {
    if (!oper && *i + _strlen(token_str) < len && 
        (_isdigit(str[*i + _strlen(token_str)]) || 
         _isalpha(str[*i + _strlen(token_str)]))) return false;
    if (_strncmp(str + *i, token_str, _strlen(token_str)) == 0) {
        struct Token token = Token_Build(token_type, line, *position, line, *position + _strlen(token_str) - 1, filename);
        TokenStream_PushBack(token_stream, token);
        *i += _strlen(token_str);
        *position += _strlen(token_str);
        return true;
    }
    else return false;
}

struct TokenStream *Lexer_Process(const char *str, const char *filename) {
    struct TokenStream *token_stream = TokenStream_New();
    int line = 0, position = 0;
    int N = _strlen(str);
    for (int i = 0; i < N;) {
        if (CheckToken(str, N, "//", i, true)) {
            i++;
            while(i < N && str[i] != '\n') i++;
            i = min(i + 1, N);
            position = 0;
            line++;
        }
        else if (CheckToken(str, N, "/*", i, true)) {
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
            i = min(i + 2, N);
        }
        else if (AppendToken(str, N, &i, "include", TokenInclude, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "if", TokenIf, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "else", TokenElse, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "while", TokenWhile, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "func", TokenFunc, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "proto", TokenProto, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "struct", TokenStruct, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "def", TokenDef, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "return", TokenReturn, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "as", TokenAs, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "const", TokenConst, false, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ":=", TokenAssign, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "<-", TokenMove, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ",", TokenComma, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ".", TokenDot, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ":", TokenColon, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ";", TokenSemicolon, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "{", TokenBraceOpen, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "}", TokenBraceClose, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "(", TokenParenthesisOpen, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ")", TokenParenthesisClose, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "[", TokenBracketOpen, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "]", TokenBracketClose, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "&", TokenAddress, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "$", TokenDereference, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "!!", TokenIndex, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "->", TokenGetField, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "^", TokenCaret, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "+", TokenPlus, true, line, &position, filename, token_stream)) {}
        else if (CheckToken(str, N, "-", i, true)) {
            if (i + 2 <= N && _isdigit(str[i + 1]) && 
               (token_stream->size == 0 || 
                   (token_stream->stream[token_stream->size - 1].type != TokenInteger && 
                    token_stream->stream[token_stream->size - 1].type != TokenIdentifier))) {
                int l = i;
                i += 2;
                while (i + 1 <= N && _isdigit(str[i])) i++;
                int r = i - 1;
                struct Token token = Token_Build(TokenInteger, line, position, line, position + r - l, filename);
                token.value_int = get_int_value(str, l, r);
                TokenStream_PushBack(token_stream, token);
                position += r - l + 1;
            }
            else AppendToken(str, N, &i, "-", TokenMinus, true, line, &position, filename, token_stream);
        }
        else if (AppendToken(str, N, &i, "*", TokenMult, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "/", TokenDiv, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "<", TokenLess, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, ">", TokenGreater, true, line, &position, filename, token_stream)) {}
        else if (AppendToken(str, N, &i, "=", TokenEqual, true, line, &position, filename, token_stream)) {}
        else if (_isdigit(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && _isdigit(str[i])) i++;
            int r = i - 1;
            struct Token token = Token_Build(TokenInteger, line, position, line, position + r - l, filename);
            token.value_int = get_int_value(str, l, r);
            TokenStream_PushBack(token_stream, token);
            position += r - l + 1;
        }
        else if (CheckToken(str, N, "\'", i, true)) {
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
                LexerError("\' expected after char", line_begin, position_begin, line, position, filename);
            }
            struct Token token = Token_Build(TokenChar, line_begin, position_begin, line, position, filename);
            token.value_int = ch;
            TokenStream_PushBack(token_stream, token);
            position++;
            i++;
        }
        else if (CheckToken(str, N, "\"", i, true)) {
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
                LexerError("\" expected after string", line_begin, position_begin, line, position, filename);
            }
            buffer[buf_len] = '\0';
            struct Token token = Token_Build(TokenString, line_begin, position_begin, line, position, filename);
            token.value_string = buffer;
            token.value_int = buf_len;
            TokenStream_PushBack(token_stream, token);
            position++;
            i++;
        }
        else if (_isalpha(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= N && (_isalpha(str[i]) || _isdigit(str[i]))) i++;
            int r = i - 1;
            struct Token token = Token_Build(TokenIdentifier, line, position, line, position + r - l, filename);
            token.value_string = _strndup(str + l, r - l + 1);
            TokenStream_PushBack(token_stream, token);
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
            LexerError("Unexpected symbol", line, position, line, position, filename);
        }
    }

    struct Token token = Token_Build(TokenEof, line, position, line, position, filename);
    TokenStream_PushBack(token_stream, token);

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
        Color_Blue,         // TokenReturn,
        Color_Blue,         // TokenAs,
        Color_Blue,         // TokenConst,
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
        Color_Black,        // TokenAddress,
        Color_Black,        // TokenDereference,
        Color_Black,        // TokenIndex,
        Color_Black,        // TokenGetField,
        Color_Black,        // TokenCaret,
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

const char *Lexer_Highlight(const char *str) {
    struct TokenStream *token_stream = Lexer_Process(str, "");
    
    int buffer_size = 1000;
    char *output = _malloc(buffer_size);
    int pos = 0;

    output[pos] = '['; pos++;
    int brace_level = 0;
    int parenthesis_level = 0;
    int bracket_level = 0;

    for (int i = 0; token_stream->stream[i].type != TokenEof; i++) {
        int cnt = sprintf(output + pos,
            "{\"lb\":%d,\"pb\":%d,\"le\":%d,\"pe\":%d,\"cl\":\"%s\"},",
            token_stream->stream[i].line_begin,
            token_stream->stream[i].position_begin,
            token_stream->stream[i].line_end,
            token_stream->stream[i].position_end,
            TokenColor(token_stream->stream[i].type,
                &brace_level,
                &parenthesis_level,
                &bracket_level));
        pos += cnt;
        if (pos + 100 >= buffer_size) {
            char *tmp = _malloc(buffer_size * 2);
            _strncpy(tmp, output, pos);
            buffer_size *= 2;
            _free(output);
            output = tmp;
        }
    }
    if (pos != 1) pos--;
    output[pos] = ']'; pos++;
    output[pos] = '\0';

    return output;
}
