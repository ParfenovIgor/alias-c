#include <exception.h>
#include <stdio.h>
#include <posix.h>

void error_lexer(const char *value, int line_begin, int position_begin, int line_end, int position_end, const char *filename) {
    _fputs(STDOUT, "Error\n");
    _fputs(STDOUT, filename);
    _fputs(STDOUT, "\n");
    _fputi(STDOUT, line_begin + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, position_begin + 1);
    _fputs(STDOUT, "-");
    _fputi(STDOUT, line_end + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, position_end + 1);
    _fputs(STDOUT, "\nLexer Error: ");
    _fputs(STDOUT, value);
    _fputs(STDOUT, "\n");
    posix_exit(1);
}

void error_syntax(const char *value, struct Token token) {
    _fputs(STDOUT, "Error\n");
    _fputs(STDOUT, token.filename);
    _fputs(STDOUT, "\n");
    _fputi(STDOUT, token.line_begin + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, token.position_begin + 1);
    _fputs(STDOUT, "-");
    _fputi(STDOUT, token.line_end + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, token.position_end + 1);
    _fputs(STDOUT, "\nSyntax Error: ");
    _fputs(STDOUT, value);
    _fputs(STDOUT, "\n");
    posix_exit(1);
}

void error_semantic(const char *value, struct Node *node) {
    _fputs(STDOUT, "Error\n");
    _fputs(STDOUT, node->filename);
    _fputs(STDOUT, "\n");
    _fputi(STDOUT, node->line_begin + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, node->position_begin + 1);
    _fputs(STDOUT, "-");
    _fputi(STDOUT, node->line_end + 1);
    _fputs(STDOUT, ":");
    _fputi(STDOUT, node->position_end + 1);
    _fputs(STDOUT, "\nSemantic Error: ");
    _fputs(STDOUT, value);
    _fputs(STDOUT, "\n");
    posix_exit(1);
}
