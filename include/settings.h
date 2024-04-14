#pragma once

#include "../include/common.h"
#include "../stdlib/include/stdbool.h"

struct Settings {
    bool states;
    bool compile;
    bool assemble;
    bool link;
    bool topMain;
    const char *inputFilename;
    const char *outputFilename;
    int outputFileDescriptor;
};

struct Settings *BuildSettings();
