#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include "common.h"
#include "token.h"
#include "ast.h"

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename);

/* class AliasException : public std::exception {
public:
    AliasException(std::string _value, int _line_begin, int _position_begin, int _line_end, int _position_end, std::string _filename) {
        value = _value;
        line_begin = _line_begin;
        position_begin = _position_begin;
        line_end = _line_end;
        position_end = _position_end;
        filename = _filename;
    }

    AliasException(std::string _value, Token _token) {
        value = _value;
        line_begin = _token.line_begin;
        position_begin = _token.position_begin;
        line_end = _token.line_end;
        position_end = _token.position_end;
        filename = _token.filename;
    }

    AliasException(std::string _value, AST::Node *_node) {
        value = _value;
        line_begin = _node->line_begin;
        position_begin = _node->position_begin;
        line_end = _node->line_end;
        position_end = _node->position_end;
        filename = _node->filename;
    }

    std::string value;
    int line_begin, position_begin, line_end, position_end;
    std::string filename;
}; */

#endif // EXCEPTION_H_INCLUDED
