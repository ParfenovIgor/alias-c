#include <exception.h>
#include <posix.h>

void LexerError(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    print_string(STDOUT, "Error\n");
    print_string(STDOUT, filename);
    print_string(STDOUT, "\n");
    print_int(STDOUT, line_begin + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, position_begin + 1);
    print_string(STDOUT, "-");
    print_int(STDOUT, line_end + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, position_end + 1);
    print_string(STDOUT, "\nLexer Error: ");
    print_string(STDOUT, value);
    print_string(STDOUT, "\n");
    posix_exit(1);
}

void SyntaxError(const char *value, struct Token token) {
    print_string(STDOUT, "Error\n");
    print_string(STDOUT, token.filename);
    print_string(STDOUT, "\n");
    print_int(STDOUT, token.line_begin + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, token.position_begin + 1);
    print_string(STDOUT, "-");
    print_int(STDOUT, token.line_end + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, token.position_end + 1);
    print_string(STDOUT, "\nSyntax Error: ");
    print_string(STDOUT, value);
    print_string(STDOUT, "\n");
    posix_exit(1);
}

void SemanticError(const char *value, struct Node *node) {
    print_string(STDOUT, "Error\n");
    print_string(STDOUT, node->filename);
    print_string(STDOUT, "\n");
    print_int(STDOUT, node->line_begin + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, node->position_begin + 1);
    print_string(STDOUT, "-");
    print_int(STDOUT, node->line_end + 1);
    print_string(STDOUT, ":");
    print_int(STDOUT, node->position_end + 1);
    print_string(STDOUT, "\nSemantic Error: ");
    print_string(STDOUT, value);
    print_string(STDOUT, "\n");
    posix_exit(1);
}
