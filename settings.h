#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "common.h"

bool Settings_GetStates();
void Settings_SetStates();
bool Settings_GetCompile();
void Settings_SetCompile(bool state);
bool Settings_GetAssemble();
void Settings_SetAssemble(bool state);
bool Settings_GetLink();
void Settings_SetLink(bool state);
bool Settings_GetTopMain();
void Settings_SetTopMain(bool state);
const char *Settings_GetFilename();
void Settings_SetFilename(const char *state);
const char *Settings_GetOutputFilename();
void Settings_SetOutputFilename(const char *state);

#endif // SETTINGS_H_INCLUDED
