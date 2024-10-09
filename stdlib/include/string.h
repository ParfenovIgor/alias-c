#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define O_RDONLY 0x0
#define O_WRONLY 0x1
#define O_RDWR   0x2
#define O_CREAT  0x50
#define O_APPEND 0x500

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
char *read_file_descriptor(int fd);
char *read_file(const char *filename);
int write_file_descriptor(int fd, const char *data);
int write_file(const char *filename, const char *data);
