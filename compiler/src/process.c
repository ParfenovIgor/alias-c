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
#include <ir.h>
#include <ir_build.h>
#include <ir_compile_x86_64.h>
#include <ir_compile.h>

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

void process_assemble(const char *input, const char *output, struct Settings *settings) {
    int pid = posix_fork();
    if (pid == 0) {
        const char *nasm = "nasm";
        const char *const args[] = {"nasm", "-f", "elf64", input, "-o", output, NULL};
        _execvp(nasm, args, 0, settings->path_variable);
    }
    posix_wait4(pid, 0, 0, 0);
}

int process(struct Settings *settings) {
    char *filename = _strdup(settings->filename_input);
    if (_strlen(filename) < 3 || _strcmp(filename + _strlen(filename) - 3, ".al") != 0) {
        _fputs(STDOUT, "The filename has to end with .al\n");
        posix_exit(1);
    }
    filename[_strlen(filename) - 3] = '\0';

    struct Node *node = process_parse(settings->filename_input, settings);

    if (settings->compile) {
        settings->filename_compile_output = settings->filename_output;
    }
    if (settings->assemble) {
        settings->filename_compile_output = _concat(_itoa(posix_getpid()), ".asm");
    }

    posix_unlink(settings->filename_compile_output);
    compile_process(node, settings);
    if (settings->compile || settings->assemble) {    
        if (settings->backend == x86_64_asm_legacy) {
            if (settings->assemble) {
                posix_unlink(settings->filename_output);
                process_assemble(settings->filename_compile_output, settings->filename_output, settings);
                posix_unlink(settings->filename_compile_output);
            }
        }
        else {
            posix_unlink(settings->filename_compile_output);
            struct IRBuilder *builder = ir_builder(settings->testing);
            ir_build(builder, node);
            posix_unlink(settings->filename_output);
            if (settings->backend == x86_64_asm) {
                ir_compile_x86_64(builder, settings->filename_output);
            }
            else {
                ir_compile(builder, settings->filename_output);
            }
        }
    }

    return 0;
}
