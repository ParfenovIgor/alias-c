#pragma once

#include <stdbool.h>

struct Settings {
    bool language_server;
    bool states;
    bool compile;
    bool assemble;
    bool link;
    const char *filename_input;
    const char *filename_output;
    const char *filename_compile_output;
};
