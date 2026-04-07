#include <lexer.h>
#include <exception.h>
#include <algorithm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

bool parse_format(char format, char *out) {
    switch (format) {
        case '0': *out = 0x0; return true;
        case 'a': *out = 0x7; return true;
        case 'b': *out = 0x8; return true;
        case 't': *out = 0x9; return true;
        case 'n': *out = 0xA; return true;
        case 'v': *out = 0xB; return true;
        case 'f': *out = 0xC; return true;
        case 'r': *out = 0xD; return true;
        case 'e': *out = 0x1B; return true;
        case '\"': *out = 0x22; return true;
        case '\'': *out = 0x27; return true;
        case '?': *out = 0x3F; return true;
        case '\\': *out = 0x5C; return true;
    }
    return false;
}

#define AppendOperator(Str, Type)                                                                   \
{                                                                                                   \
    struct Token token = token_build(Type, line, position, line, position + sizeof(Str), filename); \
    tokenstream_push(token_stream, token);                                                          \
    i += sizeof(Str) - 1;                                                                           \
    position += sizeof(Str) - 1;                                                                    \
    break;                                                                                          \
}

#define AppendKeyword(Str, Type)                                                                    \
if (_strncmp(str + i, Str, sizeof(Str) - 1) == 0 && i + sizeof(Str) <= N &&                         \
    !_isalnum(str[i + sizeof(Str) - 1]) && str[i + sizeof(Str) - 1] != '_'){                        \
    struct Token token = token_build(Type, line, position, line, position + sizeof(Str), filename); \
    tokenstream_push(token_stream, token);                                                          \
    i += sizeof(Str) - 1;                                                                           \
    position += sizeof(Str) - 1;                                                                    \
    break;                                                                                          \
}

#define Case(Char, Block) \
case Char: { \
    Block \
    goto identifier; \
}

struct TokenStream *lexer_process(const char *str, const char *filename) {
    struct TokenStream *token_stream = tokenstream_new();
    int line = 0, position = 0;
    int N = _strlen(str);
    for (int i = 0; i < N;) {
        switch (str[i]) {
            case '/': {
                if (str[i + 1] == '/') {
                    i++;
                    while (i < N && str[i] != '\n') i++;
                    i = _min(i + 1, N);
                    position = 0;
                    line++;
                    break;
                }
                if (str[i + 1] == '*') {
                    i += 2;
                    position += 2;
                    while (i + 2 <= N && (str[i] != '*' || str[i + 1] != '/')) {
                        i++;
                        position++;
                        if (str[i] == '\n') {
                            position = 0;
                            line++;
                        }
                    }
                    position += 2;
                    i = _min(i + 2, N);
                    break;
                }
                AppendOperator("/", TokenDiv);
            }
            case ':': {
                if (str[i + 1] == '=') AppendOperator(":=", TokenAssign);
                AppendOperator(":", TokenColon);
            }
            case '<': {
                if (str[i + 1] == '-') AppendOperator("<-", TokenMove);
                if (str[i + 1] == '<') AppendOperator("<<", TokenBitwiseShiftLeft);
                if (str[i + 1] == '=') AppendOperator("<=", TokenLessEqual);
                if (str[i + 1] == '>') AppendOperator("<>", TokenNotEqual);
                AppendOperator("<", TokenLess);
            }
            case '>': {
                if (str[i + 1] == '>') AppendOperator(">>", TokenBitwiseShiftRight);
                if (str[i + 1] == '=') AppendOperator(">=", TokenGreaterEqual);
                AppendOperator(">", TokenGreater);
            }
            case ',': AppendOperator(",", TokenComma);
            case '.': AppendOperator(".", TokenDot);
            case ';': AppendOperator(";", TokenSemicolon);
            case '{': AppendOperator("{", TokenBraceOpen);
            case '}': AppendOperator("}", TokenBraceClose);
            case '(': AppendOperator("(", TokenParenthesisOpen);
            case ')': AppendOperator(")", TokenParenthesisClose);
            case '[': AppendOperator("[", TokenBracketOpen);
            case ']': AppendOperator("]", TokenBracketClose);
            case '\\': AppendOperator("\\", TokenBackslash);
            case '&': AppendOperator("&", TokenAmpersand);
            case '|': AppendOperator("|", TokenPipe);
            case '^': AppendOperator("^", TokenCaret);
            case '~': AppendOperator("~", TokenTilde);
            case '$': AppendOperator("$", TokenDereference);
            case '#': AppendOperator("#", TokenSharp);
            case '@': AppendOperator("@", TokenAt);
            case '+': AppendOperator("+", TokenPlus);
            case '*': AppendOperator("*", TokenMult);
            case '%': AppendOperator("%", TokenMod);
            case '=': AppendOperator("=", TokenEqual);
            case '-': {
                if (str[i + 1] == '>') AppendOperator("->", TokenGetField);
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
                    break;
                }
                AppendOperator("-", TokenMinus);
            }
            case '\'': {
                i++;
                int line_begin = line;
                int position_begin = position;
                position++;

                char ch;
                if (str[i] == '\\' && i + 1 < N && parse_format(str[i + 1], &ch)) {
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
                break;
            }
            case '\"': {
                i++;
                int line_begin = line;
                int position_begin = position;
                position++;
                char *buffer = (char*)_malloc(1024);
                int buf_len = 0;
                while (i < N && str[i] != '\"') {
                    if (str[i] == '\\' && i + 1 < N) {
                        char ch;
                        if (parse_format(str[i + 1], &ch)) {
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
                break;
            }
            case ' ':
            case '\t':
            case '\r': {
                i++;
                position++;
                break;
            }
            case '\n': {
                i++;
                position = 0;
                line++;
                break;
            }
            Case('i', {
                AppendKeyword("include", TokenInclude);
                AppendKeyword("if", TokenIf);
            });
            Case('d', {
                AppendKeyword("defer", TokenDefer);
                AppendKeyword("def", TokenDef);
            });
            Case('e', {
                AppendKeyword("else", TokenElse);
                AppendKeyword("eval", TokenEval);
            });
            Case('w', AppendKeyword("while", TokenWhile));
            Case('f', AppendKeyword("func", TokenFunc));
            Case('p', AppendKeyword("proto", TokenProto));
            Case('s', AppendKeyword("struct", TokenStruct));
            Case('t', {
                AppendKeyword("typedef", TokenTypedef);
                AppendKeyword("test", TokenTest);
            });
            Case('r', AppendKeyword("return", TokenReturn));
            Case('b', AppendKeyword("break", TokenBreak));
            Case('c', {
                AppendKeyword("continue", TokenContinue);
                AppendKeyword("const", TokenConst);
            });
            Case('a', {
                AppendKeyword("as", TokenAs);
                AppendKeyword("and", TokenAnd);
            });
            Case('o', AppendKeyword("or", TokenOr));
            Case('n', AppendKeyword("not", TokenNot));
            default: {
                if (_isdigit(str[i])) {
                    int l = i;
                    i++;
                    while (i + 1 <= N && _isdigit(str[i])) i++;
                    int r = i - 1;
                    struct Token token = token_build(TokenInteger, line, position, line, position + r - l, filename);
                    token.value_int = get_int_value(str, l, r);
                    tokenstream_push(token_stream, token);
                    position += r - l + 1;
                }
                else if (_isalpha(str[i]) || str[i] == '_') {
                identifier:
                    int l = i;
                    i++;
                    while (i + 1 <= N && (_isalnum(str[i]) || str[i] == '_')) i++;
                    int r = i - 1;
                    struct Token token = token_build(TokenIdentifier, line, position, line, position + r - l, filename);
                    char *ptr = (char*)_malloc(r - l + 2);
                    _strncpy(ptr, str + l, r - l + 1);
                    ptr[r - l + 1] = '\0';
                    token.value_string = ptr;
                    tokenstream_push(token_stream, token);
                    position += r - l + 1;
                }
                else {
                    error_lexer("Unexpected symbol", line, position, line, position, filename);
                }
            }
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
        Color_DarkGreen,    // TokenDefer,
        Color_Blue,         // TokenIf,
        Color_Blue,         // TokenElse,
        Color_Blue,         // TokenWhile,
        Color_Blue,         // TokenFunc,
        Color_Blue,         // TokenProto,
        Color_Blue,         // TokenStruct,
        Color_Blue,         // TokenDef,
        Color_Blue,         // TokenTypedef,
        Color_Blue,         // TokenEval,
        Color_Blue,         // TokenReturn,
        Color_Blue,         // TokenBreak,
        Color_Blue,         // TokenContinue,
        Color_Blue,         // TokenAs,
        Color_Blue,         // TokenConst,
        Color_Blue,         // TokenTest,
        Color_Blue,         // TokenAnd,
        Color_Blue,         // TokenOr,
        Color_Blue,         // TokenNot,
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
        Color_Black,        // TokenAmpersand,
        Color_Black,        // TokenPipe,
        Color_Black,        // TokenCaret,
        Color_Black,        // TokenTilde,
        Color_Black,        // TokenBitwiseShiftLeft,
        Color_Black,        // TokenBitwiseShiftRight,
        Color_Black,        // TokenDereference,
        Color_Black,        // TokenGetField,
        Color_Black,        // TokenSharp,
        Color_Black,        // TokenAt,
        Color_Black,        // TokenPlus,
        Color_Black,        // TokenMinus,
        Color_Black,        // TokenMult,
        Color_Black,        // TokenDiv,
        Color_Black,        // TokenMod,
        Color_Black,        // TokenLess,
        Color_Black,        // TokenGreater,
        Color_Black,        // TokenEqual,
        Color_Black,        // TokenLessEqual,
        Color_Black,        // TokenGreaterEqual,
        Color_Black,        // TokenNotEqual,
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
