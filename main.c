#include "common.h"
#include "process.h"
#include "settings.h"

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

int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 0;
    }
    else {
        for (int i = 1; i < argc; i++) {
            const char *arg = argv[i];
            if (strcmp(arg, "-s") == 0) {
                Settings_SetStates(true);
            }
            else if(strcmp(arg, "-c") == 0) {
                Settings_SetCompile(true);
            }
            else if(strcmp(arg, "-a") == 0) {
                Settings_SetAssemble(true);
            }
            else if (strcmp(arg, "-l") == 0) {
                Settings_SetLink(true);
            }
            else if (strcmp(arg, "-m") == 0) {
                Settings_SetTopMain(true);
            }
            else if (strcmp(arg, "-o") == 0) {
                if (i + 1 == argc) {
                    print_string("Filename has to be specified after -o flag");
                    return 1;
                }
                const char *str = argv[i + 1];
                Settings_SetOutputFilename(str);
                i++;
            }
            else {
                Settings_SetFilename(arg);
            }
        }

        if (!Settings_GetFilename()) {
            help();
            return 0;
        }

        return Process();
    }
}
