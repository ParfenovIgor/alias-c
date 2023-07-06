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

int Process(struct Settings *settings) {
    struct Node *node = Parse(settings->inputFilename);

    Validate(node);
    if (settings->states) {
        PrintStatesLog();
    }

    const char *cmd;
    if (settings->compile || settings->assemble || settings->link) {
        const char *filename = settings->inputFilename;
        const char *output_filename = NULL;
        for (int i = 0; filename[i] != '\0'; i++) {
            if (filename[i] == '.') {
                output_filename = substr(filename, i);
                break;
            }
        }

        char *str = concat(output_filename, ".asm");
        FILE *file = fopen(str, "w");
        _free(str);
        Compile(node, file, settings);
        /*file.close();
        if (Settings::GetAssemble() || Settings::GetLink()) {
            cmd = "nasm -f elf32 " + filename + ".asm -o " + filename + ".o";
            system(cmd.c_str());

            if (Settings::GetLink()) {
                cmd = "gcc -m32 " + filename + ".o -no-pie -o " + filename;
                system(cmd.c_str());

                cmd = "rm " + filename + ".asm";
                system(cmd.c_str());

                cmd = "rm " + filename + ".o";
                system(cmd.c_str());

                std::string output_filename = Settings::GetOutputFilename();
                if (!output_filename.empty() && filename != output_filename) {
                    cmd = "mv " + filename + " " + output_filename;
                    system(cmd.c_str());
                }
            }
            else {
                cmd = "rm " + filename + ".asm";
                system(cmd.c_str());
                
                std::string output_filename = Settings::GetOutputFilename();
                if (!output_filename.empty() && filename + ".o" != output_filename) {
                    cmd = "mv " + filename + ".o " + output_filename;
                    system(cmd.c_str());
                }
            }
        }
        else {
            std::string output_filename = Settings::GetOutputFilename();
            if (!output_filename.empty() && filename + ".asm" != output_filename) {
                cmd = "mv " + filename + ".asm " + output_filename;
                system(cmd.c_str());
            }
        }*/
    }

    return 0;
}
