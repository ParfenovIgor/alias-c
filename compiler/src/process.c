#include <lexer.h>
#include <syntax.h>
#include <compile.h>
#include <exception.h>
#include <settings.h>
#include <process.h>
#include <posix.h>
#include <stdlib.h>
#include <string.h>

struct Node *Parse(const char *filename) {
    char *buffer = read_file(filename);
    if (!buffer) {
        print_string(STDOUT, "Could not open file ");
        print_string(STDOUT, filename);
        print_string(STDOUT, "\n");
        posix_exit(1);
    }
    struct TokenStream *token_stream = Lexer_Process(buffer, filename);
    struct Node *node = Syntax_Process(token_stream);

    return node;
}

void Assemble(const char *input, const char *output) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *nasm = "/usr/bin/nasm";
        const char *const args[] = {"/usr/bin/nasm", "-f", "elf64", input, "-o", output, NULL};
        posix_execve(nasm, args, 0);
    }
    posix_wait4(pid, 0, 0, 0);
}

void Link(const char *input, const char *output) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *gcc = "/usr/bin/ld";
        const char *const args[] = {"/usr/bin/ld", input, "-o", output, NULL};
        posix_execve(gcc, args, 0);
    }
    posix_wait4(pid, 0, 0, 0);
}

int Process(struct Settings *settings) {
    // remove the .al extension from the input filename
    char *filename = _strdup(settings->inputFilename);
    if (_strlen(filename) < 3 || _strcmp(filename + _strlen(filename) - 3, ".al") != 0) {
        print_string(STDOUT, "The filename has to end with .al\n");
        posix_exit(1);
    }
    filename[_strlen(filename) - 3] = '\0';

    // generate AST
    struct Node *node = Parse(settings->inputFilename);

    if (settings->compile || settings->assemble || settings->link) {
        // choose the filename of the result of compilation
        char *compile_out_filename;
        if (settings->compile && settings->outputFilename) {
            compile_out_filename = _strdup(settings->outputFilename);
        }
        else {
            compile_out_filename = concat(filename, ".asm");
        }

        // open file and call the compiler
        posix_unlink(compile_out_filename);
        settings->compileOutputFilename = compile_out_filename; // posix_open(compile_out_filename, 0001 | 0100, 0400 | 0200);
        Compile(node, settings);
        _free(compile_out_filename);

        if (settings->assemble || settings->link) {
            // choose the filename of the result of assembly
            char *assemble_in_filename, *assemble_out_filename;
            assemble_in_filename = concat(filename, ".asm");
            if (settings->assemble && settings->outputFilename) {
                assemble_out_filename = _strdup(settings->outputFilename);
            }
            else {
                assemble_out_filename = concat(filename, ".o");
            }

            // call the NASM assembler
            posix_unlink(assemble_out_filename);
            Assemble(assemble_in_filename, assemble_out_filename);
            posix_unlink(assemble_in_filename);
            _free(assemble_in_filename);
            _free(assemble_out_filename);

            if (settings->link) {
                // choose the filename of the result of linking
                char *link_in_filename, *link_out_filename;
                link_in_filename = concat(filename, ".o");
                if (!settings->link && settings->outputFilename) {
                    link_out_filename = _strdup(settings->outputFilename);
                }
                else {
                    link_out_filename = _strdup(filename);
                }

                // call the ld linker
                posix_unlink(link_out_filename);
                Link(link_in_filename, link_out_filename);
                posix_unlink(link_in_filename);
                _free(link_in_filename);
                _free(link_out_filename);
            }
        }
    }

    _free(filename);

    return 0;
}
