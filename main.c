#include "common.h"
#include "process.h"
#include "settings.h"
#include <unistd.h>
#include <stdio.h>

void help() {
    print_string("Syntax: calias [flags] file [flags]\n");
    print_string("Flags:\n");
    print_string("  -s        Print states collected during validation.\n");
    print_string("  -c        Compile program to Asm code.\n");
    print_string("  -a        Compile program and assemble it using nasm to object file.\n");
    print_string("  -l        Compile, assemble and link program using gcc to executable file.\n");
    print_string("  -m        Enable top level main function.\n");
    print_string("  -o        Set output file name. File name has to follow this flag.\n");
}

void test() {
    unsigned long long pid = -1;
    asm("mov $0x39, %%rax\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(pid));
    if (pid == 0) {
        const char *filename = "./aba";
        asm("mov $0x3b, %%rax\n"
            "mov %0, %%rdi\n"
            "mov $0, %%rsi\n"
            "mov $0, %%rdx\n"
            "syscall\n"
            :
            : "r"(filename)
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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 0;
    }
    else {
        struct Settings *settings = BuildSettings();
        for (int i = 1; i < argc; i++) {
            const char *arg = argv[i];
            if (strcmp(arg, "-s") == 0) {
                settings->states = true;
            }
            else if(strcmp(arg, "-c") == 0) {
                settings->compile = true;
            }
            else if(strcmp(arg, "-a") == 0) {
                settings->assemble = true;
            }
            else if (strcmp(arg, "-l") == 0) {
                settings->link = true;
            }
            else if (strcmp(arg, "-m") == 0) {
                settings->topMain = true;
            }
            else if (strcmp(arg, "-o") == 0) {
                if (i + 1 == argc) {
                    print_string("Filename has to be specified after -o flag");
                    return 1;
                }
                const char *str = _strdup(argv[i + 1]);
                settings->outputFilename = str;
                i++;
            }
            else {
                settings->inputFilename = _strdup(arg);
            }
        }

        if (!settings->inputFilename) {
            help();
            return 0;
        }

        return Process(settings);
    }
}
