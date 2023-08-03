#include "../header/exception.h"
#include "../header/posix.h"

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    print_string(0, "Error\n");
    print_string(0, filename);
    print_string(0, "\n");
    print_int(0, line_begin + 1);
    print_string(0, ":");
    print_int(0, position_begin + 1);
    print_string(0, "-");
    print_int(0, line_end + 1);
    print_string(0, ":");
    print_int(0, position_end + 1);
    print_string(0, "\nLexer Error: ");
    print_string(0, value);
    print_string(0, "\n");
    posix_exit(1);
}

void SyntaxError(const char *value, struct Token token) {
    print_string(0, "Error\n");
    print_string(0, token.filename);
    print_string(0, "\n");
    print_int(0, token.line_begin + 1);
    print_string(0, ":");
    print_int(0, token.position_begin + 1);
    print_string(0, "-");
    print_int(0, token.line_end + 1);
    print_string(0, ":");
    print_int(0, token.position_end + 1);
    print_string(0, "\nSyntax Error: ");
    print_string(0, value);
    print_string(0, "\n");
    posix_exit(1);
}
