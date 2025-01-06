#pragma once

#include <stdbool.h>
#include <vector.h>

struct Settings {
    bool language_server;
    bool validate;
    bool compile;
    bool assemble;
    bool link;
    bool testing;
    struct Vector include_names;
    struct Vector include_paths;
    const char *filename_input;
    const char *filename_output;
    const char *filename_compile_output;
    const char *path_variable;
    struct Vector included_files;
};
