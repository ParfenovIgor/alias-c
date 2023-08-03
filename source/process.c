#include "../header/common.h"
#include "../header/lexer.h"
#include "../header/syntax.h"
#include "../header/validator.h"
#include "../header/compile.h"
#include "../header/exception.h"
#include "../header/settings.h"
#include "../header/process.h"
#include "../header/posix.h"

char *ReadFile(const char *filename) {
    const int block = 1000;
    int fd = posix_open(filename, 2, 0);
    if (fd <= 0) {
        print_string(0, "Could not open file ");
        print_string(0, filename);
        print_string(0, "\n");
        posix_exit(1);
    }

    char *contents = (char*)_malloc(sizeof(char));
    contents[0] = '\0';
    int n_blocks = 0;
    while (true) {
        char *buffer = (char*)_malloc(sizeof(char) * (block + 1));
        int cnt = posix_read(fd, buffer, block);
        if (cnt == 0) break;
        buffer[cnt] = '\0';
        char *new_contents = (char*)_malloc(sizeof(char) * ((n_blocks + 1) * block + 1));
        _strcpy(new_contents, contents);
        _strcpy(new_contents + n_blocks * block, buffer);
        _free(contents);
        _free(buffer);
        contents = new_contents;
        n_blocks++;
    }

    int t = posix_close(fd);
    
    return contents;
}

struct Node *Parse(const char *filename) {
    char *buffer = ReadFile(filename);
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
    struct Node *node = Parse(settings->inputFilename);

    Validate(node);
    if (settings->states) {
        PrintStatesLog();
    }

    const char *cmd;
    if (settings->compile || settings->assemble || settings->link) {
        const char *filename = settings->inputFilename;
        const char *process_filename = NULL;
        for (int i = 0; filename[i] != '\0'; i++) {
            if (filename[i] == '.') {
                process_filename = substr(filename, i);
                break;
            }
        }

        char *str;
        if (!settings->assemble && !settings->link && settings->outputFilename) {
            str = _strdup(settings->outputFilename);
        }
        else {
            str = concat(process_filename, ".asm");
        }
        settings->outputFileDescriptor = posix_open(str, 1 | 0100, 0400 | 0200);
        _free(str);
        Compile(node, settings);
        posix_close(settings->outputFileDescriptor);
        if (settings->assemble || settings->link) {
            char *str1, *str2;
            str1 = concat(process_filename, ".asm");
            if (!settings->link && settings->outputFilename) {
                str2 = _strdup(settings->outputFilename);
            }
            else {
                str2 = concat(process_filename, ".o");
            }
            Assemble(str1, str2);
            _free(str1);
            _free(str2);

            if (settings->link) {
                char *str1, *str2;
                str1 = concat(process_filename, ".o");
                if (!settings->link && settings->outputFilename) {
                    str2 = _strdup(settings->outputFilename);
                }
                else {
                    str2 = _strdup(process_filename);
                }
                Link(str1, str2);
                _free(str1);
                _free(str2);
            }
        }
    }

    return 0;
}
