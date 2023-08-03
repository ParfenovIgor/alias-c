#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "../header/common.h"

struct Settings {
    bool states;
    bool compile;
    bool assemble;
    bool link;
    bool topMain;
    const char *inputFilename;
    const char *outputFilename;
};

struct Settings *BuildSettings();

#endif // SETTINGS_H_INCLUDED
