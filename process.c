#include "common.h"
#include "lexer.h"
#include "syntax.h"
#include "validator.h"
#include "compile.h"
#include "exception.h"
#include "settings.h"
#include "process.h"

#include <stdio.h>

struct Node *Parse(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        print_string("Could not open file ");
        print_string(filename);
        print_string("\n");
        program_exit(1);
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = (char*)_malloc(length + 1);
    int x = fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    struct TokenStream *token_stream = Lexer_Process(buffer, filename);
    struct Node *node = Syntax_Process(token_stream);

    return node;
}

void Assemble(const char *input, const char *output) {
    unsigned long long pid = -1;
    asm("mov $0x39, %%rax\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(pid));
    if (pid == 0) {
        const char *nasm = "/usr/bin/nasm";
        const char *const args[] = {"/usr/bin/nasm", "-f", "elf64", input, "-o", output, NULL};
        asm("mov $0x3b, %%rax\n"
            "mov %0, %%rdi\n"
            "mov %1, %%rsi\n"
            "mov $0, %%rdx\n"
            "syscall\n"
            :
            : "r"(nasm), "r"(args)
            : "%rax", "%rdi", "%rsi", "%rdx");
    }
    asm("mov $0x3d, %%rax\n"
        "mov %0, %%rdi\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdx\n"
        "mov $0, %%r10\n"
        "syscall\n"
        :
        : "r"(pid)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10");
}

void Link(const char *input, const char *output) {
    unsigned long long pid = -1;
    asm("mov $0x39, %%rax\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(pid));
    if (pid == 0) {
        const char *gcc = "/usr/bin/ld";
        const char *const args[] = {"/usr/bin/ld", input, "-o", output, NULL};
        asm("mov $0x3b, %%rax\n"
            "mov %0, %%rdi\n"
            "mov %1, %%rsi\n"
            "mov $0, %%rdx\n"
            "syscall\n"
            :
            : "r"(gcc), "r"(args)
            : "%rax", "%rdi", "%rsi", "%rdx");
    }
    asm("mov $0x3d, %%rax\n"
        "mov %0, %%rdi\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdx\n"
        "mov $0, %%r10\n"
        "syscall\n"
        :
        : "r"(pid)
        : "%rax", "%rdi", "%rsi", "%rdx", "%r10");
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
        FILE *file = fopen(str, "w");
        _free(str);
        Compile(node, file, settings);
        fclose(file);
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
