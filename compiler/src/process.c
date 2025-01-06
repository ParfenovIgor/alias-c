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

int _execvp(const char *filename, const char *const argv[], const char *const envp[], const char *path) {
    int n = _strlen(path);
    char full_path[1024];
    for (int l = 0; l < n;) {
        int r = l;
        while (r + 1 < n && path[r + 1] != ':') r++;

        int x = r - l + 1;
        _strncpy(full_path, path + l, x);
        full_path[x] = '/';
        _strcpy(full_path + x + 1, filename);
        
        posix_execve(full_path, argv, envp);

        l = r + 2;
    }
    
    return -1;
}

void process_assemble(const char *input, const char *output, struct Settings *settings) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *nasm = "nasm";
        const char *const args[] = {"nasm", "-f", "elf64", input, "-o", output, NULL};
        _execvp(nasm, args, 0, settings->path_variable);
    }
    posix_wait4(pid, 0, 0, 0);
}

void process_link(const char *input, const char *output, struct Settings *settings) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *ld = "ld";
        const char *const args[] = {"ld", input, "-o", output, NULL};
        _execvp(ld, args, 0, settings->path_variable);
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
            process_assemble(assemble_in_filename, assemble_out_filename, settings);
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
                process_link(link_in_filename, link_out_filename, settings);
                posix_unlink(link_in_filename);
                _free(link_in_filename);
                _free(link_out_filename);
            }
        }
    }

    _free(filename);

    return 0;
}
