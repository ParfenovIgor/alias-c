#include <process.h>
#include <settings.h>
#include <languageserver.h>
#include <string.h>
#include <stdio.h>
#include <vector.h>

void help() {
    _puts("Syntax: calias [flags] file [flags]");
    _puts("Flags:");
    _puts("  -ls                       Run language server.");
    _puts("  -s                        Print states collected during validation.");
    _puts("  -c                        Compile program to Asm code.");
    _puts("  -a                        Compile program and assemble it using nasm to object file.");
    _puts("  -l                        Compile, assemble and link program using gcc to executable file.");
    _puts("  -i <name> <path>          And include directory.");
    _puts("  -o <file>                 Set output file name.");
}

struct Settings *build_settings() {
    struct Settings *settings = (struct Settings*)_malloc(sizeof(struct Settings));
    settings->language_server = false;
    settings->states = false;
    settings->compile = false;
    settings->assemble = false;
    settings->link = false;
    settings->include_names = vnew();
    settings->include_paths = vnew();
    settings->filename_input = NULL;
    settings->filename_output = NULL;
    settings->filename_compile_output = NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 0;
    }
    else {
        struct Settings *settings = build_settings();
        for (int i = 1; i < argc; i++) {
            const char *arg = argv[i];
            if (_strcmp(arg, "-ls") == 0) {
                settings->language_server = true;
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
            else if (_strcmp(arg, "-i") == 0) {
                if (i + 2 >= argc) {
                    _puts("Name and path expected after -i flag");
                    return 1;
                }
                vpush(&settings->include_names, _strdup(argv[i + 1]));
                vpush(&settings->include_paths, _strdup(argv[i + 2]));
                i += 2;
            }
            else if (_strcmp(arg, "-o") == 0) {
                if (i + 1 == argc) {
                    _puts("Filename expected after -o flag");
                    return 1;
                }
                const char *str = _strdup(argv[i + 1]);
                settings->filename_output = str;
                i++;
            }
            else {
                settings->filename_input = _strdup(arg);
            }
        }

        if (settings->language_server) {
            language_server();
        }

        if (!settings->filename_input) {
            help();
            return 0;
        }

        return process(settings);
    }
}
