#pragma once

#include <stdbool.h>
#include <stdlib.h>

bool _isalpha(char c);
bool _isdigit(char c);

char *_strcpy(char *a, char *b);
char *_strncpy(char *a, char *b, int n);
char *_strdup(const char *a);
char *_strndup(const char *a, int n);
int _strcmp(const char *a, const char *b);
int _strncmp(const char *a, const char *b, int num);
int _strlen(const char *a);
char *concat(const char *a, const char *b);
const char *substr(const char *a, int n);
char *ReadFile(const char *filename);
