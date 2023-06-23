#include "settings.h"

bool Settings_States = false;
bool Settings_Compile = false;
bool Settings_Assemble = false;
bool Settings_Link = false;
bool Settings_TopMain = false;
const char *Settings_Filename;
const char *Settings_OutputFilename;

bool Settings_GetStates() {
    return Settings_States;
}

void Settings_SetStates(bool state) {
    Settings_States = state;
}

bool Settings_GetCompile() {
    return Settings_Compile;
}

void Settings_SetCompile(bool state) {
    Settings_Compile = state;
}

bool Settings_GetAssemble() {
    return Settings_Assemble;
}

void Settings_SetAssemble(bool state) {
    Settings_Assemble = state;
}

bool Settings_GetLink() {
    return Settings_Link;
}

void Settings_SetLink(bool state) {
    Settings_Link = state;
}

bool Settings_GetTopMain() {
    return Settings_TopMain;
}

void Settings_SetTopMain(bool state) {
    Settings_TopMain = state;
}

const char *Settings_GetFilename() {
    return Settings_Filename;
}

void Settings_SetFilename(const char *state) {
    Settings_Filename = state;
}

const char *Settings_GetOutputFilename() {
    return Settings_OutputFilename;
}

void Settings_SetOutputFilename(const char *state) {
    Settings_OutputFilename = state;
}
