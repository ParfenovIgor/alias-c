#include "exception.h"
#include "posix.h"

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    print_string("Error\n");
    print_string(filename);
    print_string("\n");
    print_int(line_begin + 1);
    print_string(":");
    print_int(position_begin + 1);
    print_string("-");
    print_int(line_end + 1);
    print_string(":");
    print_int(position_end + 1);
    print_string("\nLexer Error: ");
    print_string(value);
    print_string("\n");
    posix_exit(1);
}

void SyntaxError(const char *value, struct Token token) {
    print_string("Error\n");
    print_string(token.filename);
    print_string("\n");
    print_int(token.line_begin + 1);
    print_string(":");
    print_int(token.position_begin + 1);
    print_string("-");
    print_int(token.line_end + 1);
    print_string(":");
    print_int(token.position_end + 1);
    print_string("\nSyntax Error: ");
    print_string(value);
    print_string("\n");
    posix_exit(1);
}
