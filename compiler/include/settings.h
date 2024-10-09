#pragma once

#include <common.h>
#include <stdbool.h>

struct Settings {
    bool languageServer;
    bool states;
    bool compile;
    bool assemble;
    bool link;
    bool topMain;
    const char *inputFilename;
    const char *outputFilename;
    const char *compileOutputFilename;
};

struct Settings *BuildSettings();
