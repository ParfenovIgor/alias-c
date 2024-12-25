#include <lexer.h>
#include <syntax.h>
#include <compile.h>
#include <exception.h>
#include <settings.h>
#include <process.h>
#include <posix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct Node *process_parse(const char *filename, struct Settings *settings) {
    char *buffer = read_file(filename);
    if (!buffer) {
        _fputs(STDOUT, "Could not open file ");
        _fputs(STDOUT, filename);
        _fputs(STDOUT, "\n");
        posix_exit(1);
    }
    struct TokenStream *token_stream = lexer_process(buffer, filename);
    struct Node *node = syntax_process(token_stream, settings);

    return node;
}

struct Node *process_parse_fd(int fd, struct Settings *settings) {
    struct TokenStream *token_stream = lexer_process(read_file_descriptor(fd), "");
    struct Node *node = syntax_process(token_stream, settings);

    return node;
}

void process_assemble(const char *input, const char *output) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *nasm = "/usr/bin/nasm";
        const char *const args[] = {"/usr/bin/nasm", "-f", "elf64", input, "-o", output, NULL};
        posix_execve(nasm, args, 0);
    }
    posix_wait4(pid, 0, 0, 0);
}

void process_link(const char *input, const char *output) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *gcc = "/usr/bin/ld";
        const char *const args[] = {"/usr/bin/ld", input, "-o", output, NULL};
        posix_execve(gcc, args, 0);
    }
    posix_wait4(pid, 0, 0, 0);
}

int process(struct Settings *settings) {
    // remove the .al extension from the input filename
    char *filename = _strdup(settings->filename_input);
    if (_strlen(filename) < 3 || _strcmp(filename + _strlen(filename) - 3, ".al") != 0) {
        _fputs(STDOUT, "The filename has to end with .al\n");
        posix_exit(1);
    }
    filename[_strlen(filename) - 3] = '\0';

    // generate AST
    struct Node *node = process_parse(settings->filename_input, settings);

    if (settings->validate || settings->compile || settings->assemble || settings->link) {
        // choose the filename of the result of compilation
        char *compile_out_filename;
        if (settings->compile && settings->filename_output) {
            compile_out_filename = _strdup(settings->filename_output);
        }
        else {
            compile_out_filename = _concat(filename, ".asm");
        }

        // open file and call the compiler
        posix_unlink(compile_out_filename);
        if (!settings->validate) {
            settings->filename_compile_output = compile_out_filename;
        }
        compile_process(node, settings);
        _free(compile_out_filename);

        if (settings->assemble || settings->link) {
            // choose the filename of the result of assembly
            char *assemble_in_filename, *assemble_out_filename;
            assemble_in_filename = _concat(filename, ".asm");
            if (settings->assemble && settings->filename_output) {
                assemble_out_filename = _strdup(settings->filename_output);
            }
            else {
                assemble_out_filename = _concat(filename, ".o");
            }

            // call the NASM assembler
            posix_unlink(assemble_out_filename);
            process_assemble(assemble_in_filename, assemble_out_filename);
            posix_unlink(assemble_in_filename);
            _free(assemble_in_filename);
            _free(assemble_out_filename);

            if (settings->link) {
                // choose the filename of the result of linking
                char *link_in_filename, *link_out_filename;
                link_in_filename = _concat(filename, ".o");
                if (!settings->link && settings->filename_output) {
                    link_out_filename = _strdup(settings->filename_output);
                }
                else {
                    link_out_filename = _strdup(filename);
                }

                // call the ld linker
                posix_unlink(link_out_filename);
                process_link(link_in_filename, link_out_filename);
                posix_unlink(link_in_filename);
                _free(link_in_filename);
                _free(link_out_filename);
            }
        }
    }

    _free(filename);

    return 0;
}
