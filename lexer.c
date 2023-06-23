#include "common.h"
#include "lexer.h"
#include "exception.h"

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

struct TokenStream Lexer_Process(const char *str, const char *filename) {
    struct TokenStream token_stream;
    int line = 0, position = 0;
    int n = strlen(str);
    for (int i = 0; i < n;) {
        if (is_reserved_word(str, "asm", i)) {
            int line_begin = line;
            int position_begin = position;
            i += 3;
            position += 3;
            while (i < n && str[i] != '{') {
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == n) {
                LexerError("{ expected after asm", line_begin, position_begin, line, position, filename);
            }
            position++;
            i++;
            const char *code;
            while (i < n && str[i] != '}') {
                // code.push_back(str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == n) {
                // throw AliasException("} expected after asm", line_begin, position_begin, line, position, filename);
            }
            // token_stream.push_back(Token(TokenType::Asm, code, line_begin, position_begin, line, position, filename));
            position++;
            i++;
        }
        /*else if (is_reserved_word(str, "include", i)) {
            int line_begin = line;
            int position_begin = position;
            i += 7;
            position += 7;
            while (i < (int)str.size() && str[i] != '{') {
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == (int)str.size()) {
                throw AliasException("{ expected after include", line_begin, position_begin, line, position, filename);
            }
            position++;
            i++;
            std::string code;
            while (i < (int)str.size() && str[i] != '}') {
                code.push_back(str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == (int)str.size()) {
                throw AliasException("} expected after include", line_begin, position_begin, line, position, filename);
            }
            token_stream.push_back(Token(TokenType::Include, code, line_begin, position_begin, line, position, filename));
            position++;
            i++;
        }
        else if (is_reserved_word(str, "int", i)) {
            token_stream.push_back(Token(TokenType::Int, line, position, line, position + 2, filename));
            i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "ptr", i)) {
            token_stream.push_back(Token(TokenType::Ptr, line, position, line, position + 2, filename));
            i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "if", i)) {
            token_stream.push_back(Token(TokenType::If, line, position, line, position + 1, filename));
            i += 2;
            position += 2;
        }
        else if (is_reserved_word(str, "else", i)) {
            token_stream.push_back(Token(TokenType::Else, line, position, line, position + 3, filename));
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "while", i)) {
            token_stream.push_back(Token(TokenType::While, line, position, line, position + 4, filename));
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "func", i)) {
            token_stream.push_back(Token(TokenType::Func, line, position, line, position + 3, filename));
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "proto", i)) {
            token_stream.push_back(Token(TokenType::Proto, line, position, line, position + 4, filename));
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "def", i)) {
            token_stream.push_back(Token(TokenType::Def, line, position, line, position + 2, filename));
            i += 3;
            position += 3;
        }
        else if (is_reserved_word(str, "const", i)) {
            token_stream.push_back(Token(TokenType::Const, line, position, line, position + 4, filename));
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "assume", i)) {
            token_stream.push_back(Token(TokenType::Assume, line, position, line, position + 5, filename));
            i += 6;
            position += 6;
        }
        else if (is_reserved_word(str, "alloc", i)) {
            token_stream.push_back(Token(TokenType::Alloc, line, position, line, position + 4, filename));
            i += 5;
            position += 5;
        }
        else if (is_reserved_word(str, "free", i)) {
            token_stream.push_back(Token(TokenType::Free, line, position, line, position + 3, filename));
            i += 4;
            position += 4;
        }
        else if (is_reserved_word(str, "call", i)) {
            token_stream.push_back(Token(TokenType::Call, line, position, line, position + 3, filename));
            i += 4;
            position += 4;
        }
        else if (i + 2 <= str.size() && str.substr(i, 2) == ":=") {
            token_stream.push_back(Token(TokenType::Assign, line, position, line, position + 1, filename));
            i += 2;
            position += 2;
        }
        else if (i + 2 <= str.size() && str.substr(i, 2) == "<-") {
            token_stream.push_back(Token(TokenType::Move, line, position, line, position + 1, filename));
            i += 2;
            position += 2;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == ",") {
            token_stream.push_back(Token(TokenType::Comma, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == ":") {
            token_stream.push_back(Token(TokenType::Colon, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == ";") {
            token_stream.push_back(Token(TokenType::Semicolon, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "{") {
            token_stream.push_back(Token(TokenType::BraceOpen, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "}") {
            token_stream.push_back(Token(TokenType::BraceClose, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "(") {
            token_stream.push_back(Token(TokenType::ParenthesisOpen, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == ")") {
            token_stream.push_back(Token(TokenType::ParenthesisClose, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "[") {
            token_stream.push_back(Token(TokenType::BracketOpen, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "]") {
            token_stream.push_back(Token(TokenType::BracketClose, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "$") {
            token_stream.push_back(Token(TokenType::Dereference, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "^") {
            token_stream.push_back(Token(TokenType::Caret, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "+") {
            token_stream.push_back(Token(TokenType::Plus, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "-") {
            if (i + 2 <= str.size() && is_digit(str[i + 1]) && 
               (token_stream.empty() || (token_stream.back().type != TokenType::Integer && token_stream.back().type != TokenType::Identifier))) {
                int l = i;
                i += 2;
                while (i + 1 <= str.size() && is_digit(str[i])) i++;
                int r = i - 1;
                token_stream.push_back(Token(TokenType::Integer, atoi(str.substr(l, r - l + 1).c_str()), line, position, line, position + r - l, filename));
                position += r - l + 1;
            }
            else {
                token_stream.push_back(Token(TokenType::Minus, line, position, line, position, filename));
                i += 1;
                position += 1;
            }
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "*") {
            token_stream.push_back(Token(TokenType::Mult, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "/" && (i + 2 > str.size() || 
            (str.substr(i + 1, 1) != "/" && str.substr(i + 1, 1) != "*"))) {
            token_stream.push_back(Token(TokenType::Div, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "<") {
            token_stream.push_back(Token(TokenType::Less, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "=") {
            token_stream.push_back(Token(TokenType::Equal, line, position, line, position, filename));
            i += 1;
            position += 1;
        }
        else if (i + 2 <= str.size() && str.substr(i, 2) == "//") {
            i++;
            while(i < str.size() && str[i] != '\n') i++;
            i = std::min(i + 1, str.size());
            position = 0;
            line++;
        }
        else if (i + 2 <= str.size() && str.substr(i, 2) == "/&") {
            i += 2;
            position += 2;
            while(i + 2 <= str.size() && str.substr(i, 2) != "&/") {
                i++;
                position++;
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
            }
            position += 2;
            i = std::min(i + 2, str.size());
        }
        else if (i + 1 <= str.size() && str.substr(i, 1) == "\"") {
            i++;
            int line_begin = line;
            int position_begin = position;
            position++;
            std::string buffer;
            while (i < (int)str.size() && str[i] != '\"') {
                if (str[i] == '\\' && i + 1 < str.size()) {
                    bool found = false;
                    if (str[i + 1] == '0') {
                        buffer.push_back((char)0x0);
                        found = true;
                    }
                    if (str[i + 1] == 'a') {
                        buffer.push_back((char)0x7);
                        found = true;
                    }
                    if (str[i + 1] == 'b') {
                        buffer.push_back((char)0x8);
                        found = true;
                    }
                    if (str[i + 1] == 'e') {
                        buffer.push_back((char)0x1B);
                        found = true;
                    }
                    if (str[i + 1] == 'f') {
                        buffer.push_back((char)0xC);
                        found = true;
                    }
                    if (str[i + 1] == 'n') {
                        buffer.push_back((char)0xA);
                        found = true;
                    }
                    if (str[i + 1] == 'r') {
                        buffer.push_back((char)0xD);
                        found = true;
                    }
                    if (str[i + 1] == 't') {
                        buffer.push_back((char)0x9);
                        found = true;
                    }
                    if (str[i + 1] == 'v') {
                        buffer.push_back((char)0xB);
                        found = true;
                    }
                    if (str[i + 1] == '\\') {
                        buffer.push_back((char)0x5C);
                        found = true;
                    }
                    if (str[i + 1] == '\'') {
                        buffer.push_back((char)0x27);
                        found = true;
                    }
                    if (str[i + 1] == '\"') {
                        buffer.push_back((char)0x22);
                        found = true;
                    }
                    if (str[i + 1] == '?') {
                        buffer.push_back((char)0x3F);
                        found = true;
                    }
                    if (found) {
                        i += 2;
                        position += 2;
                        continue;
                    }
                }
                buffer.push_back(str[i]);
                if (str[i] == '\n') {
                    position = -1;
                    line++;
                }
                i++;
                position++;
            }
            if (i == (int)str.size()) {
                throw AliasException("\" expected after string", line_begin, position_begin, line, position, filename);
            }
            buffer.push_back('\0');
            token_stream.push_back(Token(TokenType::String, buffer, line_begin, position_begin, line, position, filename));
            position++;
            i++;
        }
        else if (is_digit(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= str.size() && is_digit(str[i])) i++;
            int r = i - 1;
            token_stream.push_back(Token(TokenType::Integer, atoi(str.substr(l, r - l + 1).c_str()), line, position, line, position + r - l, filename));
            position += r - l + 1;
        }
        else if (is_alpha(str[i])) {
            int l = i;
            i++;
            while (i + 1 <= str.size() && (is_alpha(str[i]) || is_digit(str[i]))) i++;
            int r = i - 1;
            token_stream.push_back(Token(TokenType::Identifier, str.substr(l, r - l + 1), line, position, line, position + r - l, filename));
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
        }*/
        else {
            //throw AliasException("Unexpected symbol", line, position, line, position, filename);
        }
    }

    //token_stream.push_back(Token(TokenType::Eof, line, position, line, position, filename));

    return token_stream;
}