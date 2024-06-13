#include "../include/common.h"
#include "../include/process.h"
#include "../include/settings.h"
#include "../include/languageserver.h"
#include "../stdlib/include/posix.h"
#include "../stdlib/include/string.h"
#include "../stdlib/include/heap.h"

void help() {
    print_string(0, "Syntax: calias [flags] file [flags]\n");
    print_string(0, "Flags:\n");
    print_string(0, "  -ls       Run language server.\n");
    print_string(0, "  -s        Print states collected during validation.\n");
    print_string(0, "  -c        Compile program to Asm code.\n");
    print_string(0, "  -a        Compile program and assemble it using nasm to object file.\n");
    print_string(0, "  -l        Compile, assemble and link program using gcc to executable file.\n");
    print_string(0, "  -m        Enable top level main function.\n");
    print_string(0, "  -o        Set output file name. File name has to follow this flag.\n");
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
            if (_strcmp(arg, "-ls") == 0) {
                settings->languageServer = true;
            }
            else if (_strcmp(arg, "-s") == 0) {
                settings->states = true;
            }
            else if(_strcmp(arg, "-c") == 0) {
                settings->compile = true;
            }
            else if(_strcmp(arg, "-a") == 0) {
                settings->assemble = true;
            }
            else if (_strcmp(arg, "-l") == 0) {
                settings->link = true;
            }
            else if (_strcmp(arg, "-m") == 0) {
                settings->topMain = true;
            }
            else if (_strcmp(arg, "-o") == 0) {
                if (i + 1 == argc) {
                    print_string(0, "Filename has to be specified after -o flag");
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

        if (settings->languageServer) {
            LanguageServer();
        }

        if (!settings->inputFilename) {
            help();
            return 0;
        }

        return Process(settings);
    }
}
