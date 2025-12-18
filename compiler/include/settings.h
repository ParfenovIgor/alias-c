#pragma once

#include <stdbool.h>
#include <vector.h>

enum Backend {
    x86_64_asm,
    c
};

struct Settings {
    bool language_server;
    bool compile;
    bool assemble;
    bool testing;
    enum Backend backend;
    struct Vector include_names;
    struct Vector include_paths;
    const char *filename_input;
    const char *filename_output;
    const char *filename_compile_output;
    const char *path_variable;
    struct Vector included_files;
};
