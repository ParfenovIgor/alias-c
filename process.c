#include "common.h"
#include "lexer.h"
#include "syntax.h"
#include "validator.h"
#include "compile.h"
#include "exception.h"
#include "settings.h"
#include "process.h"

struct Node *Parse(const char *filename) {
    /*std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Could not open file " << filename << "\n";
        exit(1);
    }

    std::stringstream buffer;
    buffer << fin.rdbuf();
    std::vector <Token> token_stream;
    std::shared_ptr <AST::Node> node;
    try {
        token_stream = Lexer::Process(buffer.str(), filename);
    }
    catch (AliasException &ex) {
        std::cout << "Error" << std::endl;
        std::cout << ex.filename << std::endl;
        std::cout << ex.line_begin + 1 << ':' << ex.position_begin + 1 << '-' << ex.line_end + 1 << ':' << ex.position_end + 1 << std::endl;
        std::cout << "Lexer Error: " << ex.value << std::endl;
        exit(1);
    }*/

    struct Node *node = Syntax_Process(0, 0);

    /*try {
        node = Syntax::Process(token_stream);
    }
    catch (AliasException &ex) {
        std::cout << "Error" << std::endl;
        std::cout << ex.filename << std::endl;
        std::cout << ex.line_begin + 1 << ':' << ex.position_begin + 1 << '-' << ex.line_end + 1 << ':' << ex.position_end + 1 << std::endl;
        std::cout << "Syntax Error: " << ex.value << std::endl;
        exit(1);
    }*/

    return node;
}

int Process() {
    struct Node *node = Parse(Settings_GetFilename());

    Validate(node);
    if (Settings_GetStates()) {
        PrintStatesLog();
    }

    const char *cmd;
    if (Settings_GetCompile() || Settings_GetAssemble() || Settings_GetLink()) {
        char *filename = (char*)Settings_GetFilename();
        for (int i = 0; filename[i] != '\0'; i++) {
            if (filename[i] == '.') {
                filename = substr(filename, i);
                break;
            }
        }

        char *str = concat(filename, ".asm");
        file_desc file = file_open(str, "w");
        string_free(str);
        /*AST::Compile(node, file);
        file.close();
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
