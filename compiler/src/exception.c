#include <exception.h>
#include <stdio.h>
#include <posix.h>

void error_lexer(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    _fputs(STDERR, "Error\n");
    _fputs(STDERR, filename);
    _fputs(STDERR, "\n");
    _fputi(STDERR, line_begin + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, position_begin + 1);
    _fputs(STDERR, "-");
    _fputi(STDERR, line_end + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, position_end + 1);
    _fputs(STDERR, "\nLexer Error: ");
    _fputs(STDERR, value);
    _fputs(STDERR, "\n");
    posix_exit(1);
}

void error_syntax(const char *value, struct Token token) {
    _fputs(STDERR, "Error\n");
    _fputs(STDERR, token.filename);
    _fputs(STDERR, "\n");
    _fputi(STDERR, token.line_begin + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, token.position_begin + 1);
    _fputs(STDERR, "-");
    _fputi(STDERR, token.line_end + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, token.position_end + 1);
    _fputs(STDERR, "\nSyntax Error: ");
    _fputs(STDERR, value);
    _fputs(STDERR, "\n");
    posix_exit(1);
}

void error_semantic(const char *value, struct Node *node) {
    _fputs(STDERR, "Error\n");
    _fputs(STDERR, node->filename);
    _fputs(STDERR, "\n");
    _fputi(STDERR, node->line_begin + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, node->position_begin + 1);
    _fputs(STDERR, "-");
    _fputi(STDERR, node->line_end + 1);
    _fputs(STDERR, ":");
    _fputi(STDERR, node->position_end + 1);
    _fputs(STDERR, "\nSemantic Error: ");
    _fputs(STDERR, value);
    _fputs(STDERR, "\n");
    posix_exit(1);
}
