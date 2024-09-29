#include <common.h>
#include <process.h>
#include <settings.h>
#include <languageserver.h>
#include <posix.h>
#include <string.h>
#include <stdio.h>
#include <heap.h>

void help() {
    _puts("Syntax: calias [flags] file [flags]");
    _puts("Flags:");
    _puts("  -ls       Run language server.");
    _puts("  -s        Print states collected during validation.");
    _puts("  -c        Compile program to Asm code.");
    _puts("  -a        Compile program and assemble it using nasm to object file.");
    _puts("  -l        Compile, assemble and link program using gcc to executable file.");
    _puts("  -m        Enable top level main function.");
    _puts("  -o        Set output file name. File name has to follow this flag.");
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
                    _puts("Filename has to be specified after -o flag");
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
