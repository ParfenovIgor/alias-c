#include "exception.h"

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
    program_exit(0);
}
