#include "../header/settings.h"
#include "../header/common.h"

struct Settings *BuildSettings() {
    struct Settings *settings = (struct Settings*)_malloc(sizeof(struct Settings));
    settings->states = false;
    settings->compile = false;
    settings->assemble = false;
    settings->link = false;
    settings->topMain = false;
    settings->inputFilename = NULL;
    settings->outputFilename = NULL;
}
